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
	
	refcount_ptr(dummy_class *in) {
		// only for NULL initialization
		assert(in == NULL);
		this->pointer = NULL;
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
	refcount_ptr(const refcount_ptr<T> &rhs) {
		this->pointer = ~rhs;
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
class unique_ptr {
private:
	T *pointer;
public:
	unique_ptr() {
		this->pointer = NULL;
	}
	unique_ptr(T *rhs) {
		this->pointer = rhs;
	}
	
	unique_ptr operator=(const T *&rhs) {
		delete this->pointer;
		this->pointer = rhs;
	}

	T *operator->() {
		return pointer;
	}
	T *operator*() {
		return pointer;
	}
	T &operator[](int i) {
		return pointer[i];
	}

	~unique_ptr() {
		// no need to check for NULL - delete automatically ignores it
		delete this->pointer;
	}
};
