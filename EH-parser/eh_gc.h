/*
 * eh_gc.cpp
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
 */
#include <list>

#define FOREACH(container, varname) for(container::iterator varname = container.begin(), _end = container.end(); varname != end; varname++)
template<class T>
class garbage_collector {
private:
	/*
	 * constants
	 */
	const static int pool_size = 512;

	/*
	 * types
	 */
	class block {
	public:
		short refcount;
		short gc_data;
		// could we make an explicit union here? That might fail because T is not a POD. In any case, this is implicitly a union of a T and a block*
		T content;
		
		// get the next pointer from a free block. Havoc will result if this is called on an allocated block.
		block *get_next_pointer() const {
			assert(!this->is_allocated());
			return (block *)&this->content;
		}
		void set_next_pointer(block *in) {
			*(block *)&this->content = in;
		}
		
		bool is_allocated() const {
			return this->refcount == 0;
		}
		// set bits, numbered from 0 to 15
		void set_gc_bit(int bit) {
			this->gc_data |= (1 << (15 - bit));
		}
		void unset_gc_bit(int bit) {
			this->gc_data &= ~(1 << (15 - bit));		
		}
		bool get_gc_bit(int bit) const {
			return (bool) this->gc_data & (1 << (15 - bit));
		}
	};

	class pool {
		// pointer to next pool in the list
		pool *next;
		
		// pointer to the first free block in the list
		block* first_free_block;
		// number of free blocks left
		int free_blocks;

		T blocks[pool_size];
		
		pool(pool *_next = NULL) : next(_next), first_free_block(NULL), free_blocks(pool_size), pool() {}
		
		~pool() {
			if(next != NULL) {
				delete next;
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
			b->refcount = 0;
			b->content.~T();
			b->refcount = 0;
			b->gc_data = 0;
			b->set_next_pointer(this->first_free_block);
			this->first_free_block = b;
			free_blocks++;
		}
	};
	
	class marking_bit {
	private:
		int value;
	public:
		int get() const {
			assert(this->value >= 0 && this->value < sizeof(short));
			return this->value;
		}
		void inc() {
			this->value++;
			if(this->value == sizeof(short)) {
				this->value = 0;
			}
		}
		int next() const {
			return this->value % sizeof(short);
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

public:
	class pointer {
	private:
		block *content;
		
		block *operator~() {
			return this->content;
		}
		// TODO: pretty much the same as refcount_ptr; perhaps we can combine them.
	public:
		T &operator*() const {
			if(this->content == NULL) {
//				this->pointer = 
			}
			return this->content->content;
		}
		T *operator->() const {
			if(this->content == NULL) {
//				this->pointer = new container<T>;	
			}
			return &this->content->content;
		}
		
		static bool null(pointer in) {
			return ~in == NULL;
		}
		
		friend class garbage_collector;
	};

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
	
	/*
	 * private methods
	 */
	block *real_allocate() {
		// find pool
		if(this->current_pool->full()) {
			this->find_current_pool();
		}
		return current_pool->alloc();
	}
	
	// Allocate a block in the given pool. This cannot be a member of pool because it needs access to global GC state.
	block *alloc(pool *p) {
		assert(!p->full());
		block *out = p->first_free_block;
		block *next = p->first_free_block->get_next_pointer();
		if(next == NULL) {
			// it's the next block
			p->first_free_block++;
		} else {
			p->first_free_block = next;
		}
		p->free_blocks--;
		
		// create the object
		T *obj = new(&out->content) T();
		// first set GC bit, then tell GC that this has been allocated
		out->set_gc_bit(this->marking_bit.get());
		out->set_gc_bit(this->marking_bit.next());
		out->refcount = 1;
		return out;
	}
	
	void find_current_pool() {
		for(pool *p = this->first_pool; p != NULL; p = p->next) {
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
	void do_mark(pointer root) {
		root.set_gc_bit(this->marking_bit.get());
		// not sure whether this will compile
		std::list<pointer> children = root->children();
		for(typename std::list<pointer>::iterator i = children.begin(), end = children.end(); i != end; i++) {
			this->do_mark(i);
		}
	}
	
	// Remove all blocks with no references to them found by do_mark().
	void do_sweep() {
		int current_bit = this->marking_bit.get();
		int previous_bit = this->marking_bit.prev();
		for(pool *p = this->first_pool, *prev = NULL; p != NULL; prev = p, p = p->next) {
			for(int i = 0; i < pool_size; i++) {
				block b = p->blocks[i];
				if(b->is_allocated() && !b->get_gc_bit(current_bit)) {
					p->dealloc(b);
				} else {
					// unset old GC bits
					p->unset_gc_bit(previous_bit);
				}
				// GC pools
				if(p->empty()) {
					if(prev == NULL) {
						this->first_pool = p->next;
					} else {
						prev->next = p->next;
					}
					if(p == this->current_pool) {
						this->current_pool = this->find_current_pool();
					}
					delete p;
				}
			}
		}
	}
public:
	// public methods
	pointer allocate() {
		// this doesn't actually allocate anything: it just pretends to
		return pointer();
	}
	
	void do_collect(pointer root) {
		this->do_mark(root);
		this->marking_bit.inc();
		this->do_sweep();
	}

	// constructors and destructors
	garbage_collector() : first_pool(new pool), current_pool(first_pool), current_bit() {
		// otherwise our strategy won't work
		assert(sizeof(T) >= sizeof(void *));
	}
	
	~garbage_collector() {
		delete first_pool;
	}
};
