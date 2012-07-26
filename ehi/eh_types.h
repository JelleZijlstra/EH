#include <tr1/unordered_map>

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
			case array_e:
			case binding_e:
			case hash_e:
			case tuple_e:
			case range_e:
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
		struct ehfunc_t *funcval;
		struct ehrange_t *rangeval;
		// pseudo-types for internal use
		struct opnode_t *opval;
		type_enum typeval;
		attribute_enum attributeval;
		attributes_t attributestrval;
		class LibraryBaseClass *resourceval;
		struct ehbinding_t *bindingval;
		struct ehhash_t *hashval;
		void *base_objectval;
		class ehtuple_t *tupleval;
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
	EHRV_SET(struct ehfunc_t *, func)
	EHRV_SET(type_enum, type)
	EHRV_SET(class LibraryBaseClass *, resource)
	EHRV_SET(ehbinding_t *, binding)
	EHRV_SET(struct ehhash_t *, hash)
	EHRV_SET(class ehtuple_t *, tuple)
#undef EHRV_SET
	// special constructors for GC'ed types
#define EHRV_GC(vtype, ehtype) static void fill_ ## ehtype(ehretval_p in, vtype val) { \
	in->type(ehtype ## _e); \
	in->ehtype ## val = val; \
}
	EHRV_GC(ehobj_t *, object)
	EHRV_GC(ehobj_t *, weak_object)
	EHRV_GC(eharray_t *, array)
	EHRV_GC(ehbinding_t *, binding)
	EHRV_GC(ehrange_t *, range)
	EHRV_GC(struct ehhash_t *, hash)
	EHRV_GC(struct ehfunc_t *, func)
	EHRV_GC(class ehtuple_t *, tuple)
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
			COPY(binding);
			COPY(hash);
			COPY(base_object);
			COPY(tuple);
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
			case object_e: case weak_object_e:
				return true;
			default:
				return false;
		}
	}
	
	ehobj_t *get_object() const {
		assert(this->is_object());
		return this->objectval;
	}
	
	bool is_a(int in);
	
	void print();
	bool equals(ehretval_p rhs);
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
typedef array_ptr<ehretval_p> ehretval_a;

// context
typedef ehretval_p /* with type object_e */ ehcontext_t;

typedef ehretval_p (*ehlibmethod_t)(ehretval_p, ehretval_p, ehcontext_t, class EHI *);

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

// EH object
typedef struct ehobj_t {
public:
	// typedefs
	typedef std::map<const std::string, ehmember_p> obj_map;
	typedef obj_map::iterator obj_iterator;
	
	// properties
	obj_map members;
	// the object's state data
	ehretval_p object_data;
	// the type
	int type_id;
	// for scoping
	ehretval_p parent;
	ehretval_p real_parent;
	// destructor needs it
	EHI *ehi;

	// constructors
	ehobj_t() : members(), object_data(NULL), type_id(null_e), parent(), real_parent() {}

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
	void set(const char *name, ehretval_p value);
	void set(std::string name, ehretval_p value);

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
	~ehobj_t();
private:
	ehmember_p get_recursive_helper(const char *name, const ehcontext_t context);
} ehobj_t;
#define OBJECT_FOR_EACH(obj, varname) for(ehobj_t::obj_iterator varname = (obj)->members.begin(), end = (obj)->members.end(); varname != end; varname++)

/*
 * EH functions. Unlike other primitive types, functions must always be wrapped
 * in objects in order to preserve scope.
 */
typedef struct ehfunc_t {
	functype_enum type;
	int argcount;
	eharg_t *args;
	ehretval_p code;
	ehlibmethod_t libmethod_pointer;
	
	ehfunc_t(functype_enum _type = user_e) : type(_type), argcount(0), args(NULL), code(), libmethod_pointer(NULL) {}
	
	// we own the args thingy
	~ehfunc_t() {
		if(args != NULL) {
			delete[] args;
		}
	}
} ehfunc_t;

// range
typedef struct ehrange_t {
	ehretval_p min;
	ehretval_p max;
	
	ehrange_t(ehretval_p _min, ehretval_p _max) : min(_min), max(_max) {
		assert(min->type() == max->type());
	}
	ehrange_t() : min(ehretval_t::make_int(0)), max(ehretval_t::make_int(0)) {}
} ehrange_t;

// method binding
typedef struct ehbinding_t {
	ehretval_p object_data;
	ehretval_p method;

	ehbinding_t(ehretval_p _object_data, ehretval_p _method) : object_data(_object_data),  method(_method) {}
} ehbinding_t;

// hash
typedef struct ehhash_t {
private:
	typedef std::map<std::string, ehretval_p> hash;
	hash members;
public:
	typedef hash::iterator hash_iterator;
	
	ehhash_t() : members() {}
	
	bool has(const char *key) {
		return members.count(key);
	}
	void set(const char *key, ehretval_p value) {
		members[key] = value;
	}
	ehretval_p get(const char *key) {
		return members[key];
	}
	
	hash_iterator begin_iterator() {
		return members.begin();
	}
	hash_iterator end_iterator() {
		return members.end();
	}
} ehhash_t;
#define HASH_FOR_EACH(obj, varname) for(ehhash_t::hash_iterator varname = (obj)->begin_iterator(), end = (obj)->end_iterator(); varname != end; varname++)

class type_repository {
private:
	std::map<int, std::string> id_to_string;
	std::map<int, ehretval_p> id_to_object;
	int next_available;

	// the size of type_enum
	const static int first_user_type = 19;
public:
	
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
	
	std::string get_name(int id) {
		if(id_to_string.count(id) == 1) {
			return id_to_string[id];
		} else {
			assert(false);
			return NULL;
		}
	}
	ehretval_p get_object(int id) {
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

// EH tuples
class ehtuple_t {
private:
  const int _size;
  ehretval_a content;
public:
  ehtuple_t(int size, ehretval_p *in) : _size(size), content(size) {
    for(int i = 0; i < size; i++) {
      content[i] = in[i];
    }
  }
  
  int size() const {
    return this->_size;
  }
  ehretval_p get(int i) const {
    assert(i >= 0 && i < _size);
    return this->content[i];
  }
};
