// EH value, and generic node
typedef struct ehretval_t {
private:
	type_enum _type;
	void type(type_enum type) {
		this->_type = type;
	}
	
	static bool belongs_in_gc(type_enum type) {
		switch(type) {
			case object_e:
			case weak_object_e:
			case func_e:
			case array_e:
				return true;
			default:
				return false;
		}	
	}
public:
	typedef garbage_collector<ehretval_t>::pointer ehretval_p;
	//typedef refcount_ptr<ehretval_t> ehretval_p;
	union {
		// simple EH variable type
		int intval;
		char *stringval;
		bool boolval;
		float floatval;
		// complex types
		struct eharray_t *arrayval;
		struct ehobj_t *objectval;
		struct ehobj_t *weak_objectval;
		struct ehobj_t *funcval;
		struct ehrange_t *rangeval;
		// pseudo-types for internal use
		struct opnode_t *opval;
		type_enum typeval;
		attribute_enum attributeval;
		attributes_t attributestrval;
		void *resourceval;
	};
	// constructors
	ehretval_t() : _type(null_e) {}
	ehretval_t(type_enum type) : _type(type), stringval(NULL) {}
	// overloaded factory method; should only be used if necessary (e.g., in eh.y)
#define EHRV_MAKE(vtype, ehtype) static ehretval_p make(vtype in) { \
	ehretval_p out; \
	out->type(ehtype ## _e); \
	out->ehtype ## val = in; \
	return out; \
}
	EHRV_MAKE(int, int)
	EHRV_MAKE(char *, string)
	EHRV_MAKE(bool, bool)
	EHRV_MAKE(float, float)
	EHRV_MAKE(struct opnode_t *, op)
	EHRV_MAKE(type_enum, type)
#undef EHRV_MAKE
#define EHRV_SET(vtype, ehtype) static ehretval_p make_ ## ehtype(vtype in) { \
	ehretval_p out; \
	if(belongs_in_gc(ehtype ## _e)) { \
		assert(false); \
	} \
	out->type(ehtype ## _e); \
	out->ehtype ## val = in; \
	return out; \
} \
vtype get_ ## ehtype ## val() const { \
	assert(this->type() == ehtype ## _e); \
	return this->ehtype ## val; \
}
	EHRV_SET(int, int)
	EHRV_SET(char *, string)
	EHRV_SET(bool, bool)
	EHRV_SET(float, float)
	EHRV_SET(struct eharray_t *, array)
	EHRV_SET(struct ehobj_t *, object)
	EHRV_SET(struct ehrange_t *, range)
	EHRV_SET(struct opnode_t *, op)
	EHRV_SET(attribute_enum, attribute)
	EHRV_SET(attributes_t, attributestr)
	EHRV_SET(struct ehobj_t *, weak_object)
	EHRV_SET(struct ehobj_t *, func)
	EHRV_SET(type_enum, type)
	EHRV_SET(void *, resource)
#undef EHRV_SET
	// special constructors for GC'ed types
#define EHRV_GC(vtype, ehtype) static void fill_ ## ehtype(ehretval_p in, vtype val) { \
	in->type(ehtype ## _e); \
	in->ehtype ## val = val; \
}
	EHRV_GC(ehobj_t *, object)
	EHRV_GC(ehobj_t *, weak_object)
	EHRV_GC(ehobj_t *, func)
	EHRV_GC(eharray_t *, array)
#undef EHRV_GC

	void overwrite(ehretval_t &in) {
		this->type(in.type());
		switch(this->_type) {
#define COPY(type) case type ## _e: this->type ## val = in.type ## val; break
			COPY(int);
			COPY(string);
			COPY(bool);
			COPY(float);
			COPY(array);
			case weak_object_e:
			COPY(object);
			COPY(func);
			COPY(range);
			COPY(op);
			COPY(type);
			COPY(attribute);
			COPY(attributestr);
			COPY(resource);
			case null_e: break;
#undef COPY
		}
	}
	
	// other methods
	type_enum type() const {
		if(this == NULL) {
			return null_e;
		} else {
			return this->_type;
		}
	}
	
	bool belongs_in_gc() const {
		return belongs_in_gc(this->type());
	}
	
	bool is_object() const {
	  switch(this->type()) {
	    case object_e: case weak_object_e: case func_e:
	      return true;
	    default:
	      return false;
	  }
	}
	
	ehobj_t *get_object() const {
	  assert(this->is_object());
	  return this->objectval;
	}
	
	void print();
	std::list<ehretval_p> children();
	
	~ehretval_t();
	
	static ehretval_p make_typed(type_enum type) {
		ehretval_p out;
		if(belongs_in_gc(type)) {
			// can't do that here
			assert(false);
		}
		out->type(type);
		// this should NULL the union
		out->stringval = NULL;
		return out;
	}
	static ehretval_p make(ehretval_p in) {
		return in;
	}
} ehretval_t;

typedef ehretval_t::ehretval_p ehretval_p;

// context
typedef ehretval_p /* with type object_e */ ehcontext_t;

typedef void *(*ehconstructor_t)();
typedef void (*ehdestructor_t)(ehobj_t *);

typedef ehretval_p (*ehlibmethod_t)(ehretval_p, int, ehretval_p *, ehcontext_t, class EHI *);

// Variables and object members (which are the same)
typedef struct ehmember_t {
	attributes_t attribute;
	ehretval_p value;

	// destructor
	~ehmember_t() {
	}
	
	ehmember_t() : value() {
		attribute.visibility = public_e;
		attribute.isstatic = nonstatic_e;
		attribute.isconst = nonconst_e;
	}
	ehmember_t(attributes_t atts) : attribute(atts), value() {}
	
	// convenience methods
	bool isstatic() {
		return this->attribute.isstatic == static_e;
	}
	bool isconst() {
		return this->attribute.isconst == const_e;
	}
} ehmember_t;
typedef refcount_ptr<ehmember_t> ehmember_p;

// in future, add type for type checking
typedef struct eharg_t {
	std::string name;
} eharg_t;

// EH array
typedef struct eharray_t {
	// typedefs
	typedef std::map<const int, ehretval_p> int_map;
	typedef std::map<const std::string, ehretval_p> string_map;
	typedef std::pair<const int, ehretval_p>& int_pair;
	typedef std::pair<const std::string, ehretval_p>& string_pair;
	typedef int_map::iterator int_iterator;
	typedef string_map::iterator string_iterator;

	// properties
	int_map int_indices;
	string_map string_indices;
	
	// constructor
	eharray_t() : int_indices(), string_indices() {}
	
	// inline methods
	size_t size() const {
		return this->int_indices.size() + this->string_indices.size();
	}
	
	bool has(ehretval_p index) const {
		switch(index->type()) {
			case int_e: return this->int_indices.count(index->get_intval());
			case string_e: return this->string_indices.count(index->get_stringval());
			default: return false;
		}
	}
	
	// methods
	ehretval_p &operator[](ehretval_p index);
	void insert_retval(ehretval_p index, ehretval_p value);
} eharray_t;
#define ARRAY_FOR_EACH_STRING(array, varname) for(eharray_t::string_iterator varname = (array)->string_indices.begin(), end = (array)->string_indices.end(); varname != end; varname++)
#define ARRAY_FOR_EACH_INT(array, varname) for(eharray_t::int_iterator varname = (array)->int_indices.begin(), end = (array)->int_indices.end(); varname != end; varname++)

// struct with common infrastructure for procedures and methods
typedef struct ehfm_t {
	functype_enum type;
	int argcount;
	eharg_t *args;
	ehretval_p code;
  ehlibmethod_t libmethod_pointer;
	
	ehfm_t(functype_enum _type) : type(_type), argcount(0), args(NULL), code(), libmethod_pointer(NULL) {}
	ehfm_t() : type(user_e), argcount(0), args(NULL), code(), libmethod_pointer(NULL) {}
	
	// we own the args thingy
	~ehfm_t() {
		if(args != NULL) {
			delete[] args;
		}
	}
} ehfm_t;
typedef refcount_ptr<ehfm_t> ehfm_p;

// EH object
typedef struct ehobj_t {
public:
	// typedefs
	typedef std::map<const std::string, ehmember_p> obj_map;
	typedef obj_map::iterator obj_iterator;
	
	// properties
	obj_map members;
	ehfm_p function;
	std::string classname;
	ehretval_p parent;
	ehretval_p real_parent;
	// for library classes
	ehretval_p object_data;
	ehconstructor_t constructor;
	ehdestructor_t destructor;

	// constructors
	ehobj_t(std::string _classname) : members(), function(), classname(_classname), parent(), real_parent(), object_data(NULL), constructor(NULL), destructor(NULL) {}

	// method prototypes
	ehmember_p insert_retval(const char *name, attributes_t attribute, ehretval_p value);
	ehmember_p get_recursive(const char *name, const ehcontext_t context, int token);
	ehmember_p get(const char *name, const ehcontext_t context, int token);
	void copy_member(obj_iterator &classmember, bool set_real_parent, ehretval_p ret, EHI *ehi);
	bool context_compare(const ehcontext_t key) const;

	// inline methods
	size_t size() const {
		return members.size();
	}
	
	// set with default attributes
	void set(const char *name, ehretval_p value) {
	  if(this->has(name)) {
	    this->members[name]->value = value;
	  } else {
	    ehmember_p newmember;
	    newmember->attribute = attributes_t::make(public_e, nonstatic_e, nonconst_e);
	    newmember->value = value;
	    this->members[name] = newmember;
	  }
	}

	void insert(const std::string &name, ehmember_p value) {
		members[name] = value;
	}
	void insert(const char *name, ehmember_p value) {
		const std::string str(name);
		this->insert(str, value);
	}
	
	ehmember_p &operator[](std::string key) {
		return members[key];
	}
	
	bool has(const std::string key) const {
		return members.count(key);
	}
	
	struct ehobj_t *get_parent() const {
		if(ehretval_p::null(this->parent)) {
			return NULL;
		} else {
			return this->parent->get_objectval();
		}
	}
	
	struct ehobj_t *get_real_parent() const {
		if(ehretval_p::null(this->real_parent)) {
			return NULL;
		} else {
			return this->real_parent->get_objectval();
		}
	}
	
	// destructor
	~ehobj_t() {
		if(this->object_data->type() == resource_e) {
			this->destructor(this);
		}
	}
private:
	ehmember_p get_recursive_helper(const char *name, const ehcontext_t context);
} ehobj_t;
#define OBJECT_FOR_EACH(obj, varname) for(ehobj_t::obj_iterator varname = (obj)->members.begin(), end = (obj)->members.end(); varname != end; varname++)

// range
typedef struct ehrange_t {
	int min;
	int max;
	
	ehrange_t(int _min, int _max) : min(_min), max(_max) {}
	ehrange_t() : min(0), max(0) {}
} ehrange_t;
