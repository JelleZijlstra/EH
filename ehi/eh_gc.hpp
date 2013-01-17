/*
 * eh_gc.h
 * Jelle Zijlstra, July 2012
 *
 * A tracing garbage collector for specific C++ types.
 *
 * This garbage collector takes care of a specific pool of memory, consisting of
 * n blocks that can hold a single object of type T plus a word's worth of
 * meta-information. In the beginning, the entire pool is set to 0, indicating
 * that the blocks are free.
 *
 * Free blocks always have their meta-information set to all 0. They can either
 * have the space for the object also set to 0, which indicates that the next
 * free block is the one immediately following this block, or they can contain a
 * pointer to the next available free block.
 *
 * Occupied blocks have a non-zero refcount. They are periodically swept by the
 * garbage collector.
 *
 * There are only three basic operations: creating a garbage_collector object,
 * requesting it to create a new T object, and requesting it to do a GC run.
 *
 * Something to think about: if we have circular references, what will happen
 * when we free them here and something that refers back to them tries to
 * decrease the refcount of the thing we freed in the first place? I think we
 * should set the refcount to 0 before calling the destructor; that way, the
 * reference count will perhaps fall to -1 or so, but we don't care. After the
 * destructor is done, we set the refcount to 0 again and put the block in the
 * free list.
 *
 * Another potential problem is the GC root. What if we have objects that have
 * valid references on the stack, but that aren't yet reachable from the GC
 * root? We may be able to solve this to some extent by only running the GC
 * between statements; even then, stuff like eh_op_declareclass may be in
 * trouble.
 *
 * When I experimentally ran the GC after every statement (the T_SEPARATOR
 * case in eh_execute), it crashed horribly, presumably because of this reason.
 *
 * Types included in this GC should have a method belongs_in_gc that tells the
 * GC whether a particular object should be allocated within the GC-checked
 * part of memory or within the normal heap.
 *
 * New solution (January 2013): Add a concept of strong references (i.e.,
 * references from the stack). These references prevent the GC from
 * collecting the block. Thus, there must be two kinds of pointers: one
 * strong, one weak.
 */
#include <list>
#include "concurrency.hpp"

template<class T>
class garbage_collector {
private:
	/*
	 * constants
	 */
	const static int pool_size = 512;
	const static int self_freed = 1;

	const static int assignmetns_between_runs = 4096;

	/*
	 * types
	 */
	class block;
public:
	class data {
	public:
		int refcount;
		unsigned short strong_refcount;
		short gc_data;

		// get the next pointer from a free block. Havoc will result if this is called on an allocated block.
		class garbage_collector::block *get_next_pointer() {
			assert(!this->is_allocated());
			char *ptr = reinterpret_cast<char *>(this) + sizeof(data);
			return *reinterpret_cast<class garbage_collector::block **>(ptr);
		}
		void set_next_pointer(void *in) {
			char *ptr = reinterpret_cast<char *>(this) + sizeof(data);
			*reinterpret_cast<void **>(ptr) = in;
		}

		void inc_rc() {
			this->refcount++;
			if(this->refcount == 0) {
				std::cerr << "Refcount reached 0 after increase!" << std::endl;
			}
		}
		void inc_strong_rc() {
			this->strong_refcount++;
			if(this->strong_refcount == 0) {
				std::cerr << "Strong-refcount reached 0 after increase!" << std::endl;
			}
		}

		void dec_rc() {
			// this may happen if we free a still-referenced object as unreachable
			if(this->refcount == 0) {
				return;
			}
			this->refcount--;
			if(this->refcount == 0) {
				if(this->belongs_in_gc()) {
					// We can free this block now, but there is no way to let
					// the	pool know that this block is free. Thus, we instead
					// set the next_pointer to 1, and the sweeper will
					// understand that this is a self-freed block.
					this->suicide();
					this->set_next_pointer(reinterpret_cast<void *>(self_freed));
				} else {
					delete this;
				}
			}
		}
		void dec_strong_rc() {
			this->strong_refcount--;
		}

		// set bits, numbered from 0 to 15
		void set_gc_bit(int bit) {
			this->gc_data |= (1 << (15 - bit));
		}
		void unset_gc_bit(int bit) {
			this->gc_data &= ~(1 << (15 - bit));
		}
		bool get_gc_bit(int bit) const {
			return static_cast<bool>(this->gc_data & (1 << (15 - bit)));
		}

		bool is_allocated() const {
			return this->refcount != 0;
		}

		bool has_strong_refs() const {
			return strong_refcount > 0;
		}

		void suicide() {
			// needed to avoid bugs with the destructor indirectly
			// leading to the refcount for this block being lowered
			this->refcount = 0;
			this->~data();
			this->refcount = 0;
			this->strong_refcount = 0;
			this->gc_data = 0;
		}

		// don't set the gc_data - it's set by garbage_collector code below
		data() : refcount(1), strong_refcount(1) {}

		virtual ~data() {}

		virtual bool belongs_in_gc() const {
			return false;
		}
	};

private:
	class block {
	public:
		T content;
		// needed so we can use this for classes inheriting from T
		void *padding;

		data *get_data() {
			return reinterpret_cast<data *>(&this->content);
		}

		const data *get_data() const {
			return reinterpret_cast<const data *>(&this->content);
		}

		// get the next pointer from a free block. Havoc will result if this is called on an allocated block.
		block *get_next_pointer() {
			return this->get_data()->get_next_pointer();
		}
		void set_next_pointer(block *in) {
			this->get_data()->set_next_pointer(in);
		}

		bool is_self_freed() {
			return this->get_next_pointer() == reinterpret_cast<block *>(self_freed);
		}
		bool is_allocated() const {
			return this->get_data()->is_allocated();
		}
		bool has_strong_refs() const {
			return this->get_data()->has_strong_refs();
		}
	};

	class pool {
	private:
		pool(const pool&) = delete;
		pool operator=(const pool&) = delete;
	public:
		// pointer to next pool in the list
		pool *next;

		// pointer to the first free block in the list
		block *first_free_block;
		// number of free blocks left
		int free_blocks;

		uint8_t blocks[pool_size * sizeof(block)];

		pool(pool *_next = nullptr) : next(_next), first_free_block(nullptr), free_blocks(pool_size), blocks() {
			first_free_block = reinterpret_cast<block *>(&blocks[0]);
		}

		~pool() {
			assert(free_blocks == pool_size);
		}

		void flush() {
			if(free_blocks < pool_size) {
				// kill everything
				for(int i = 0; i < pool_size; i++) {
					block *b = (block *)&this->blocks[i * sizeof(block)];
					if(b->is_allocated()) {
						this->dealloc(b);
					} else if(b->is_self_freed()) {
						this->harvest_self_freed(b);
					}
					// if pool is now empty, no need to continue sweep
					if(this->empty()) {
						break;
					}
				}
			}
		}

		// methods
		bool full() const {
			return free_blocks == 0;
		}
		bool empty() const {
			return free_blocks == pool_size;
		}

		void dealloc(block *b) {
#ifdef DEBUG_GC_MORE
			std::cout << "Freeing block at " << b << std::endl;
			std::cout << "Refcount: " << b->refcount << std::endl;
			b->content.print();
#endif /* DEBUG_GC_MORE */
			b->get_data()->suicide();
			b->set_next_pointer(this->first_free_block);
			this->first_free_block = b;
			free_blocks++;
		}

		void harvest_self_freed(block *b) {
#ifdef DEBUG_GC_MORE
			std::cout << "Harvesting block at " << b << std::endl;
#endif /* DEBUG_GC_MORE */
			assert(b->is_self_freed());
			b->set_next_pointer(this->first_free_block);
			this->first_free_block = b;
			free_blocks++;
		}

		block *get_block(int i) {
			return reinterpret_cast<block *>(&blocks[i * sizeof(block)]);
		}

		void sweep(int previous_bit, int new_bit) {
			for(int i = 0; i < pool_size; i++) {
				block *b = get_block(i);
				if(b->is_allocated() && !b->get_data()->get_gc_bit(new_bit)) {
					this->dealloc(b);
				} else {
					// unset old GC bits
					b->get_data()->unset_gc_bit(previous_bit);
					// assimilate self-freed blocks
					if(!b->is_allocated() && b->is_self_freed()) {
						this->harvest_self_freed(b);
					}
				}
				// if pool is now empty, no need to continue sweep
				if(this->empty()) {
					break;
				}
			}
		}
	};

	class marking_bit {
	private:
		int value;
	public:
		int get() const {
			assert(this->value >= 0 && ((unsigned) this->value) < sizeof(short));
			return this->value;
		}
		void inc() {
			this->value++;
			if(this->value == sizeof(short)) {
				this->value = 0;
			}
		}
		int next() const {
			return (this->value + 1) % sizeof(short);
		}
		int prev() const {
			if(this->value == 0) {
				return sizeof(short) - 1;
			} else {
				return this->value - 1;
			}
		}

		marking_bit() : value(0) {}
	};

	/*
	 * forbidden operations
	 */
	garbage_collector(const garbage_collector &);
	garbage_collector operator=(const garbage_collector &);

public:
	template<bool is_strong>
	class pointer {
	private:
		mutable T *content;

		void inc_rc() {
			this->content->inc_rc();
			if(is_strong) {
				this->content->inc_strong_rc();
			}
		}

		void dec_rc() {
			this->content->dec_rc();
			if(is_strong) {
				this->content->dec_strong_rc();
			}
		}
	public:
		/*
		 * Constructors
		 */
		pointer() : pointer(nullptr) {}
		// my compiler apparently doesn't have std::nullptr_t
		pointer(decltype(nullptr)) : pointer(T::null_object()) {}
		pointer(const pointer &rhs) : content(rhs.content) {
			assert(content != nullptr);
			inc_rc();
		}
		pointer(T *in) : content(in) {
			assert(content != nullptr);
			inc_rc();
		}

		pointer(const pointer<!is_strong> &rhs) : content(rhs.content) {
			inc_rc();
		}
		/*
		 * Overloading
		 */
		T &operator*() const {
			return *(this->operator->());
		}
		T *operator->() const {
			if(this->content == nullptr) {
				return T::null_object().content;
			} else {
				return this->content;
			}
		}
		pointer &operator=(const pointer &rhs) {
			// decrease refcount for thing we're now referring to
			if(this->content != nullptr) {
				this->dec_rc();
			}
			this->content = rhs.content;
			// and increase it for what we're now referring to
			if(this->content != nullptr) {
				this->inc_rc();
			}
			return *this;
		}

		/*
		 * So that it can be used in a map
		 */
		int compare(const pointer &rhs) const {
			if(this->content == rhs.content) {
				return 0;
			} else if(this->null()) {
				return -1;
			} else if(rhs.null()) {
				return 1;
			} else {
				return this->content->compare(rhs);
			}
		}
		bool operator<(const pointer &rhs) const {
			return this->compare(rhs) == -1;
		}

		bool operator==(const pointer &rhs) const {
			return this->compare(rhs) == 0;
		}

		bool operator!=(const pointer &rhs) const {
			return this->compare(rhs) != 0;
		}

		bool operator>(const pointer &rhs) const {
			return this->compare(rhs) == 1;
		}

		bool null() const {
			return this->content == nullptr;
		}

		/*
		 * Destructor
		 */
		~pointer() {
			if(this->content != nullptr) {
				this->dec_rc();
			}
		}

		friend class garbage_collector;
	};

	typedef pointer<true> strong_pointer;
	typedef pointer<false> weak_pointer;

private:
	/*
	 * private properties
	 */

	// first pool in the linked list
	pool *first_pool;
	// pool we're currently allocating from
	pool *current_pool;
	// current bit used for marking by the GC
	marking_bit current_bit;
	// number of assignments made since last GC run
	unsigned int assignments_since_last_run;

	/*
	 * private methods
	 */
	data *real_allocate() {
		// find pool
		if(this->current_pool->full()) {
			this->find_current_pool();
		}
		pool *p = current_pool;
		assert(!p->full());
		block *out = p->first_free_block;
		block *next = p->first_free_block->get_next_pointer();
		if(next == nullptr) {
			// it's the next block
			p->first_free_block++;
		} else {
			p->first_free_block = next;
		}
		p->free_blocks--;

		data *d = out->get_data();
		// first set GC bit, then tell GC that this has been allocated
		d->set_gc_bit(this->current_bit.get());
		d->set_gc_bit(this->current_bit.next());

		// increment number of assignments
		assignments_since_last_run++;
		run_if_necessary();
		return d;
	}

	void find_current_pool() {
		for(pool *p = this->first_pool; p != nullptr; p = p->next) {
			if(!p->full()) {
				this->current_pool = p;
				return;
			}
		}
		// we didn't find a pool with space, so create a new one
		this->first_pool = new pool(this->first_pool);
		this->current_pool = this->first_pool;
	}

	/*
	 * Garbage collection.
	 */
	void do_mark_with_root(weak_pointer root) {
		int bit = this->current_bit.next();
		// ignore already marked objects and objects that are not in GC
		if(root == nullptr || !root->belongs_in_gc() || root->get_gc_bit(bit)) {
			return;
		}
		root->set_gc_bit(bit);

		auto children = root->children();
		// std::cout << "Size of children: " << children.size() << std::endl;
		// std::cout << (typeid(root.operator*())).name() << std::endl;
 		for(auto &i : children) {
			do_mark_with_root(i);
		}
	}

	void do_mark() {
		for(pool *p = first_pool; p != nullptr; p = p->next) {
			for(int i = 0; i < pool_size; i++) {
				auto b = p->get_block(i);
				if(b->has_strong_refs()) {
					do_mark_with_root(&b->content);
				}
			}
		}
	}

	// Remove all blocks with no references to them found by do_mark().
	void do_sweep() {
		int new_bit = this->current_bit.get();
		int previous_bit = this->current_bit.prev();
		for(pool *p = this->first_pool; p != nullptr; p = p->next) {
			p->sweep(previous_bit, new_bit);
		}
		// remove pools that are now empty
		for(pool *p = this->first_pool, *prev = nullptr; p != nullptr; prev = p, p = p->next) {
			if(p->empty()) {
				if(prev == nullptr) {
					this->first_pool = p->next;
				} else {
					prev->next = p->next;
				}
				if(p == this->current_pool) {
					this->find_current_pool();
				}
				delete p;
				if(prev == nullptr) {
					p = this->first_pool;
				} else {
					p = prev;
				}
			}
		}
	}

	void run_if_necessary() {
		if(assignments_since_last_run > assignmetns_between_runs) {
			do_collect();
		}
	}
public:
	data *get_space() {
		return real_allocate();
	}

	void do_collect() {
#ifdef DEBUG_GC
		std::cout << "Starting GC run..." << std::endl;
		print_stats();
#endif /* DEBUG_GC */
		do_mark();
		current_bit.inc();
		do_sweep();
#ifdef DEBUG_GC
		std::cout << "Done" << std::endl;
		print_stats();
#endif /* DEBUG_GC */
		// reset
		assignments_since_last_run = 0;
	}

	void print_stats() const {
		int num_pools = 0;
		int allocated_blocks = 0;
		for(pool *p = this->first_pool; p != nullptr; p = p->next) {
			num_pools++;
			allocated_blocks += (pool_size - p->free_blocks);
		}
		std::cout << "Number of pools: " << num_pools << std::endl;
		std::cout << "Allocated blocks: " << allocated_blocks << std::endl;
	}

	// constructors and destructors
	garbage_collector() : first_pool(new pool), current_pool(first_pool), current_bit(), assignments_since_last_run(0) {
		// otherwise our strategy won't work
		assert(sizeof(T) >= sizeof(void *));
	}

	~garbage_collector() {
		for(pool *p = this->first_pool; p != nullptr; p = p->next) {
			p->flush();
		}

		pool *next = nullptr;
		for(pool *p = this->first_pool; p != nullptr; p = next) {
			next = p->next;
			delete p;
		}
	}
};
