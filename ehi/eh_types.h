#include <set>

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
			case array_e:
			case binding_e:
			case hash_e:
			case tuple_e:
			case super_class_e:
			case range_e:
				return true;
			default:
				return false;
		}	
	}
	union {
		// simple EH variable type
		int intval;
		char *stringval;
		bool boolval;
		float floatval;
		// complex types
		class eharray_t *arrayval;
		struct ehobj_t *objectval;
		class ehfunc_t *funcval;
		class ehrange_t *rangeval;
		// pseudo-types for internal use
		struct opnode_t *opval;
		type_enum typeval;
		attribute_enum attributeval;
		attributes_t attributestrval;
		class LibraryBaseClass *resourceval;
		class ehbinding_t *bindingval;
		class ehhash_t *hashval;
		void *base_objectval;
		class ehtuple_t *tupleval;
		class ehsuper_t *super_classval;
	};
public:
	typedef garbage_collector<ehretval_t>::pointer ehretval_p;
	//typedef refcount_ptr<ehretval_t> ehretval_p;
	// constructors
	ehretval_t() : _type(null_e), stringval(NULL) {}
	ehretval_t(type_enum type) : _type(type), stringval(NULL) {}
	// overloaded factory method; should only be used if necessary (e.g., in eh.y)
#define EHRV_MAKE(vtype, ehtype) static ehretval_p make(vtype in) { \
	ehretval_p out; \
	out->type(ehtype ## _e); \
	out->ehtype ## val = in; \
	return out; \
}
	EHRV_MAKE(type_enum, type)
	EHRV_MAKE(attribute_enum, attribute)
	EHRV_MAKE(int, int)
	EHRV_MAKE(char *, string)
	EHRV_MAKE(bool, bool)
	EHRV_MAKE(float, float)
	EHRV_MAKE(struct opnode_t *, op)
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
vtype get_ ## ehtype ## val() const;
	EHRV_SET(int, int)
	EHRV_SET(char *, string)
	EHRV_SET(bool, bool)
	EHRV_SET(float, float)
	EHRV_SET(class eharray_t *, array)
	EHRV_SET(struct ehobj_t *, object)
	EHRV_SET(class ehrange_t *, range)
	EHRV_SET(struct opnode_t *, op)
	EHRV_SET(attribute_enum, attribute)
	EHRV_SET(attributes_t, attributestr)
	EHRV_SET(class ehfunc_t *, func)
	EHRV_SET(type_enum, type)
	EHRV_SET(class LibraryBaseClass *, resource)
	EHRV_SET(ehbinding_t *, binding)
	EHRV_SET(class ehhash_t *, hash)
	EHRV_SET(class ehtuple_t *, tuple)
	EHRV_SET(class ehsuper_t *, super_class)
#undef EHRV_SET
	// special constructors for GC'ed types
#define EHRV_GC(vtype, ehtype) static void fill_ ## ehtype(ehretval_p in, vtype val) { \
	in->type(ehtype ## _e); \
	in->ehtype ## val = val; \
}
	EHRV_GC(ehobj_t *, object)
	EHRV_GC(eharray_t *, array)
	EHRV_GC(ehbinding_t *, binding)
	EHRV_GC(ehrange_t *, range)
	EHRV_GC(class ehhash_t *, hash)
	EHRV_GC(class ehfunc_t *, func)
	EHRV_GC(class ehtuple_t *, tuple)
	EHRV_GC(class ehsuper_t *, super_class)
#undef EHRV_GC

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

	int get_full_type() const;
	const std::string &type_string(class EHI *ehi) const;

	// Compare two ehretval_ps (guaranteed to be of the same type)
	int naive_compare(ehretval_p rhs) {
		if(this->type() == null_e) {
			return 0;
		}
		void *lhs_val = reinterpret_cast<void *>(this->objectval);
		void *rhs_val = reinterpret_cast<void *>(rhs->objectval);
		if(lhs_val < rhs_val) {
			return -1;
		} else if(lhs_val == rhs_val) {
			return 0;
		} else {
			return 1;
		}
	}
	
	bool is_object() const {
		return this->type() == object_e;
	}
	
	ehobj_t *get_object() const {
		assert(this->is_object());
		return this->objectval;
	}
	
	static ehretval_p self_or_data(const ehretval_p in);
	
	bool is_a(int in);
	bool inherited_is_a(int in);
	
	void print();
	bool equals(ehretval_p rhs);
	std::list<ehretval_p> children();
	ehretval_p instantiate(EHI *ehi);
	
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
typedef array_ptr<ehretval_p> ehretval_a;

// context
struct ehcontext_t {
	ehretval_p object;
	ehretval_p scope;

	ehcontext_t(ehretval_p _object, ehretval_p _scope) : object(_object), scope(_scope) {}

	ehcontext_t(ehretval_p in) : object(in), scope(in) {}

	ehcontext_t() : object(), scope() {}
};

typedef ehretval_p (*ehlibmethod_t)(ehretval_p, ehretval_p, class EHI *);

// Variables and object members (which are the same)
typedef struct ehmember_t {
	typedef refcount_ptr<ehmember_t> ehmember_p;

	attributes_t attribute;
	ehretval_p value;

	// destructor
	~ehmember_t() {
	}
	
	ehmember_t() : attribute(attributes_t::make()), value() {}
	ehmember_t(attributes_t atts) : attribute(atts), value() {}
	
	// convenience methods
	bool isstatic() {
		return this->attribute.isstatic == static_e;
	}
	bool isconst() {
		return this->attribute.isconst == const_e;
	}

	static ehmember_p make(attributes_t attribute, ehretval_p value) {
		ehmember_p out;
		out->attribute = attribute;
		out->value = value;
		return out;
	}
} ehmember_t;
typedef ehmember_t::ehmember_p ehmember_p;

// EH object
typedef struct ehobj_t {
public:
	// typedefs
	typedef std::map<const std::string, ehmember_p> obj_map;
	typedef obj_map::iterator obj_iterator;
	typedef void (*initializer)(ehobj_t *obj, EHI *ehi);
	
	// properties
	obj_map members;
	// the object's state data
	ehretval_p object_data;
	// the type
	int type_id;
	// for scoping
	ehretval_p parent;
	ehretval_p real_parent;
	// inheritance
	std::list<ehretval_p> super;
	// destructor needs it
	EHI *ehi;

	// constructors
	ehobj_t() : members(), object_data(), type_id(null_e), parent(), real_parent(), super(), ehi() {}

	// method prototypes
	ehmember_p get_recursive(const char *name, const ehcontext_t context);
	bool context_compare(const ehcontext_t key) const;

	bool inherited_has(const std::string &key) const;
	ehmember_p inherited_get(const std::string &key);
	
	std::set<std::string> member_set();

	// inline methods
	size_t size() const {
		return members.size();
	}
	
	void insert(const std::string &name, ehmember_p value) {
		members[name] = value;
	}
	void insert(const char *name, ehmember_p value) {
		const std::string str(name);
		this->insert(str, value);
	}
	
	ehmember_p get_known(const std::string &key) {
		assert(this->has(key));
		return members[key];
	}

	bool has(const std::string &key) const {
		return members.count(key);
	}
	bool has(const char *key) const {
		return has(std::string(key));
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

	void inherit(ehretval_p superclass) {
		super.push_front(superclass);
	}
	
	void register_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, EHI *ehi);
	
	void register_member_class(const std::string &name, const int type_id, const ehobj_t::initializer init_func, const attributes_t attributes, EHI *ehi, ehretval_p the_class = NULL);
	
	// destructor
	~ehobj_t();
private:
	// ehobj_t should never be handled directly
	ehobj_t(const ehobj_t&);
	ehobj_t operator=(const ehobj_t&);
} ehobj_t;
#define OBJECT_FOR_EACH(obj, varname) for(ehobj_t::obj_iterator varname = (obj)->members.begin(), end = (obj)->members.end(); varname != end; varname++)

class type_repository {
private:
	std::map<int, std::string> id_to_string;
	std::map<int, ehretval_p> id_to_object;
	int next_available;

public:
	// the size of type_enum
	const static int first_user_type = 19;
	
	void register_known_class(int id, std::string name, ehretval_p object) {
		assert(id < first_user_type);
		id_to_string[id] = name;
		id_to_object[id] = object;
	}
	
	int register_class(std::string name, ehretval_p object) {
		id_to_string[next_available] = name;
		id_to_object[next_available] = object;
		next_available++;
		return next_available - 1;
	}
	
	const std::string &get_name(int id) {
		// pretend bindings are just functions
		if(id == binding_e) {
			id = func_e;
		}
		if(id_to_string.count(id) == 1) {
			return id_to_string[id];
		} else {
			assert(false);
			throw std::exception();
		}
	}
	ehretval_p get_object(int id) {
		if(id == binding_e) {
			id = func_e;
		}
		if(id_to_object.count(id) == 1) {
			return id_to_object[id];
		} else {
			assert(false);
			return NULL;
		}
	}
	
	type_repository() : id_to_string(), id_to_object(), next_available(first_user_type) {}
};

class eh_exception : public std::exception {
public:
	ehretval_p content;

	eh_exception(ehretval_p _content) : content(_content) {}

	virtual ~eh_exception() throw() {}
};

// define methods
#define EHRV_SET(vtype, ehtype) inline vtype ehretval_t::get_ ## ehtype ## val() const { \
	if(this->type() == object_e && ehtype ## _e != object_e) { \
		return this->get_objectval()->object_data->get_ ## ehtype ## val(); \
	} \
	assert(this->type() == ehtype ## _e); \
	return this->ehtype ## val; \
}
EHRV_SET(int, int)
EHRV_SET(char *, string)
EHRV_SET(bool, bool)
EHRV_SET(float, float)
EHRV_SET(class eharray_t *, array)
EHRV_SET(struct ehobj_t *, object)
EHRV_SET(class ehrange_t *, range)
EHRV_SET(struct opnode_t *, op)
EHRV_SET(attribute_enum, attribute)
EHRV_SET(attributes_t, attributestr)
EHRV_SET(class ehfunc_t *, func)
EHRV_SET(type_enum, type)
EHRV_SET(class LibraryBaseClass *, resource)
EHRV_SET(ehbinding_t *, binding)
EHRV_SET(class ehhash_t *, hash)
EHRV_SET(class ehtuple_t *, tuple)
EHRV_SET(class ehsuper_t *, super_class)
