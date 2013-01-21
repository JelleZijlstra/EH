#include <algorithm>

template<class T>
class refcount_ptr {
private:
	class container {
	public:
		short refcount;
		T content;

		template<typename... Args>
		container(Args&&... args) : refcount(1), content(std::forward<Args>(args)...) {}

		void inc_rc() {
			this->refcount++;
		}

		void dec_rc() {
			this->refcount--;
			if(this->refcount == 0) {
				delete this;
			}
		}
	};

	mutable container* pointer;

public:
	bool null() const {
		return this->pointer == nullptr;
	}

	// constructor
	// refcount_ptr() : pointer(nullptr) {}

	explicit refcount_ptr(T *in) {
		assert(in != nullptr);
		this->pointer = new container();
		this->pointer->content = *in;
	}

	refcount_ptr(decltype(nullptr)) : pointer(nullptr) {}

	refcount_ptr(const refcount_ptr &rhs) : pointer(rhs.pointer) {
		if(pointer != nullptr) {
			pointer->inc_rc();
		}
	}

	template<class... Args>
	explicit refcount_ptr(Args&&... args) : pointer(new container(std::forward<Args>(args)...)) {}

	T &operator*() const {
		return this->pointer->content;
	}
	T *operator->() const {
		return &this->pointer->content;
	}
	refcount_ptr &operator=(const refcount_ptr &rhs) {
		// decrease refcount for thing we're now referring to
		if(this->pointer != nullptr) {
			this->pointer->dec_rc();
		}
		this->pointer = rhs.pointer;
		// and increase it for what we're now referring to
		if(this->pointer != nullptr) {
			this->pointer->inc_rc();
		}
		return *this;
	}

	bool operator==(const refcount_ptr &rhs) {
		return this->pointer == rhs.pointer;
	}
	bool operator==(void *rhs) {
		return (void *)this->pointer == rhs;
	}
	bool operator!=(const refcount_ptr &rhs) {
		return this->pointer != rhs.pointer;
	}
	bool operator!=(void *rhs) {
		return (void *)this->pointer != rhs;
	}

	~refcount_ptr() {
		if(this->pointer != nullptr) {
			this->pointer->dec_rc();
		}
	}
};

template<class T>
class array_ptr {
private:
	T *pointer;

	// disallowed operations
	array_ptr(const array_ptr&);
	array_ptr operator=(const array_ptr&);
public:
	// Only provide this constructor, since others would be risky
	array_ptr(unsigned int n) : pointer() {
		if(n > 0) {
			this->pointer = new T[n]();
		}
	}

	T &operator[](unsigned int i) {
		return pointer[i];
	}

	T operator[](unsigned int i) const {
	  return pointer[i];
	}

	operator T *() {
		return pointer;
	}

	~array_ptr() {
		// no need to check for nullptr - delete automatically ignores it
		delete[] this->pointer;
	}
};
