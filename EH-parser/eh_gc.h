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
				//delete this;
			}
		}
	};

	mutable container<T>* pointer;
	
	// this to make sure we can internally access pointer
	container<T>* operator~() const {
		return this->pointer;
	}
	
	// need both a reference and a non-reference one
	container<T>* &operator!() const {
		return this->pointer;
	}
	
public:
	static refcount_ptr<T> clone(refcount_ptr<T> in) {
		refcount_ptr<T> out;
		if(~in != NULL) {
			!out = new container<T>(*~in);
		}
		return out;		
	}
	static bool null(refcount_ptr<T> in) {
		return ~in == NULL;
	}
	
	// constructor
	refcount_ptr() : pointer(NULL) {}
	refcount_ptr(T *in) {
		if(in == NULL) {
			this->pointer = NULL;
		} else {
			this->pointer = new container<T>;
			this->pointer->content = *in;
		}
	}
	
	// TODO: fix warnings in g++4.5 about converting NULL to int. One solution is to have a private "dummy_class" here and have a constructor that takes a dummy_class* argument (https://groups.google.com/forum/?fromgroups#!topic/comp.lang.c++/4pIZb6Glxa4). However, that creates an ambiguity with the current T* constructor. That will no longer be an issue when we can kill that constructor (i.e., if we no longer have ehretval_t * flying around in the AST).
	refcount_ptr(int in) {
		if(in == 0) {
			this->pointer = NULL;
		} else {
			throw 0;
		}
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
		if(~rhs == NULL) {
			!rhs = new container<T>;
		}
		this->pointer = ~rhs;
		// and increase it for what we're now referring to
		this->pointer->inc_rc();
		return *this;
	}
	refcount_ptr(const refcount_ptr<T> &rhs) {
		if(~rhs == NULL) {
			!rhs = new container<T>;
		}
		this->pointer = ~rhs;
		this->pointer->inc_rc();
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

/*	// share this value with a new variable
	static refcount_ptr<T> share(refcount_ptr<T> in) {
	 	container<T>* con = ~in;
		// can share if it's already shared or if there is only one reference
		if(con->is_shared || (con->refcount == 1)) {
			con->inc_rc();
			con->is_shared++;
			return in;
		} else {
			return clone(in);
		}
	}*/
/* I think this is unnecessary.
	// make a reference to this object, overwriting the ehretval_t * pointed to by in.
	ehretval_t *reference(ehretval_t *&in) {
		if(is_shared == 0) {
			inc_rc();
			return this;
		} else {
			ehretval_t *out = clone();
			is_shared--;
			dec_rc();
			in = out;
			// one for the reference, one for the actual object
			out->inc_rc();
			return out;
		}
	}*
	void make_shared() {
		is_shared++;
	}*/

};
