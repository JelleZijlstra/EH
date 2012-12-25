template<class T>
class refcount_ptr {
private:
	template<class I>
	class container {
	public:
		short refcount;
		short shared;
		I content;
		
		container() : refcount(1), shared(0), content() {}
		
		void inc_rc() {
			this->refcount++;
		}
		
		void dec_rc() {
			this->refcount--;
			if(this->shared != 0) {
				this->shared--;
			}
			if(this->refcount == 0) {
				delete this;
			}
		}
	};

	mutable container<T>* pointer;
	
	// this to make sure we can internally access pointer
	container<T>* &operator~() const {
		return this->pointer;
	}
	
	class dummy_class {};
public:
	static refcount_ptr<T> clone(refcount_ptr<T> in) {
		refcount_ptr<T> out;
		if(~in != NULL) {
			~out = new container<T>(*~in);
		}
		return out;		
	}
	static bool null(refcount_ptr<T> in) {
		return ~in == NULL;
	}
	bool null() const {
		return this->pointer == NULL;
	}
	
	// constructor
	refcount_ptr() : pointer(NULL) {}
	explicit refcount_ptr(T *in) {
		if(in == NULL) {
			this->pointer = NULL;
		} else {
			this->pointer = new container<T>;
			this->pointer->content = *in;
		}
	}
	
	refcount_ptr(dummy_class *in) : pointer(NULL) {
		// only for NULL initialization
		assert(in == NULL);
	}

	T &operator*() const {
		if(this->pointer == NULL) {
			this->pointer = new container<T>;	
		}
		return this->pointer->content;
	}
	T *operator->() const {
		if(this->pointer == NULL) {
			this->pointer = new container<T>;	
		}
		return &this->pointer->content;
	}
	refcount_ptr<T> &operator=(const refcount_ptr<T> &rhs) {
		// decrease refcount for thing we're now referring to
		if(this->pointer != NULL) {
			this->pointer->dec_rc();
		}
		this->pointer = ~rhs;
		// and increase it for what we're now referring to
		if(this->pointer != NULL) {
			this->pointer->inc_rc();
		}
		return *this;
	}
	refcount_ptr(const refcount_ptr<T> &rhs) : pointer(~rhs) {
		if(this->pointer != NULL) {
			this->pointer->inc_rc();
		}
	}

	bool operator==(const refcount_ptr<T> &rhs) {
		return this->pointer == ~rhs;
	}
	bool operator==(void *rhs) {
		return (void *)this->pointer == rhs;
	}
	bool operator!=(const refcount_ptr<T> &rhs) {
		return this->pointer != ~rhs;
	}
	bool operator!=(void *rhs) {
		return (void *)this->pointer != rhs;
	}

	~refcount_ptr() {
		if(this->pointer != NULL) {
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
	array_ptr(int n) : pointer() {
		if(n > 0) {
			this->pointer = new T[n]();
		}
	}
	
	T &operator[](int i) {
		return pointer[i];
	}
	
	T operator[](int i) const {
	  return pointer[i];
	}
	
	operator T *() {
		return pointer;
	}

	~array_ptr() {
		// no need to check for NULL - delete automatically ignores it
		delete[] this->pointer;
	}
};
