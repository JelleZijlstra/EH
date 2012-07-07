/*
 * eh.h
 * Jelle Zijlstra, December 2011–May 2012
 *
 * Main header file for the EH scripting language
 */
#ifndef EH_H_
#define EH_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <exception>
#include <iostream>

#include "eh_gc.h"

#include <map>
#include <iostream>
#include <algorithm>

/*
 * Enums used in the parser and interpreter
 */

typedef enum type_enum {
	null_e = 0,
	string_e = 1,
	int_e = 2,
	bool_e = 3,
	accessor_e = 4, // for internal use with array/object accessors
	type_e = 5, // for internal use with type casting
	array_e,
	func_e, // methods
	object_e,
	op_e,
	attribute_e,
	attributestr_e,
	range_e,
	float_e
} type_enum;

// attributes of class members
typedef enum visibility_enum {
	public_e = 0,
	private_e = 1
} visibility_enum;

typedef enum static_enum {
	nonstatic_e = 0,
	static_e = 1
} static_enum;

typedef enum const_enum {
	nonconst_e = 0,
	const_e = 1
} const_enum;

// struct for class member attributes
typedef struct attributes_t {
	visibility_enum visibility : 2;
	static_enum isstatic : 1;
	const_enum isconst : 1;
	
	// can't make a constructor because this thing appears in a union, but this
	// is almost as good
	static attributes_t make(visibility_enum v, static_enum s, const_enum c) {
		attributes_t out;
		out.visibility = v;
		out.isstatic = s;
		out.isconst = c;
		return out;
	}
} attributes_t;

// and accompanying enum used by the parser
typedef enum attribute_enum {
	publica_e,
	privatea_e,
	statica_e,
	consta_e
} attribute_enum;

typedef enum functype_enum {
	user_e = 0,
	lib_e = 1,
	libmethod_e = 2
} functype_enum;

typedef enum accessor_enum {
	arrow_e,
	doublecolon_e
} accessor_enum;

/*
 * Parser and interpreter structs
 */

// EH value, and generic node
typedef struct ehretval_t {
private:
	type_enum _type;
public:
	typedef refcount_ptr<ehretval_t> ehretval_p;
	union {
		// simple EH variable type
		int intval;
		char *stringval;
		bool boolval;
		float floatval;
		// complex types
		struct eharray_t *arrayval;
		struct ehobj_t *objectval;
		struct ehobj_t *funcval;
		struct ehrange_t *rangeval;
		// pseudo-types for internal use
		struct opnode_t *opval;
		type_enum typeval;
		attribute_enum attributeval;
		attributes_t attributestrval;
		accessor_enum accessorval;
	};
	// constructors
	ehretval_t() : _type(null_e) {}
	ehretval_t(type_enum type) : _type(type), stringval(NULL) {}
#define EHRV_CONS(vtype, ehtype) ehretval_t(vtype in) : _type(ehtype ## _e), ehtype ## val(in) {}
	EHRV_CONS(int, int)
	EHRV_CONS(char *, string)
	EHRV_CONS(bool, bool)
	EHRV_CONS(float, float)
	EHRV_CONS(double, float)
	EHRV_CONS(struct eharray_t *, array)
	EHRV_CONS(struct ehobj_t *, object)
	EHRV_CONS(struct ehrange_t *, range)
#undef EHRV_CONS
#define EHRV_SET(vtype, ehtype) void set(vtype in) { \
	this->type(ehtype ## _e); \
	this->ehtype ## val = in; \
} \
static ehretval_p make(vtype in) { \
	ehretval_p out; \
	out->type(ehtype ## _e); \
	out->set(in); \
	return out; \
}
	EHRV_SET(int, int)
	EHRV_SET(char *, string)
	EHRV_SET(bool, bool)
	EHRV_SET(float, float)
	EHRV_SET(double, float)
	EHRV_SET(struct eharray_t *, array)
	EHRV_SET(struct ehobj_t *, object)
	EHRV_SET(struct ehrange_t *, range)
	EHRV_SET(struct opnode_t *, op)
	EHRV_SET(accessor_enum, accessor)
	EHRV_SET(attribute_enum, attribute)
#undef EHRV_SET

	void overwrite(ehretval_t &in) {
		this->type(in.type());
		switch(this->_type) {
#define COPY(type) case type ## _e: this->type ## val = in.type ## val; break
			COPY(int);
			COPY(string);
			COPY(bool);
			COPY(float);
			COPY(array);
			COPY(object);
			COPY(func);
			COPY(range);
			COPY(op);
			COPY(type);
			COPY(attribute);
			COPY(attributestr);
			COPY(accessor);
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
	void type(type_enum type) {
		this->_type = type;
	}
	void print();
	
	~ehretval_t();
	
	static ehretval_p make(type_enum type) {
		ehretval_p out;
		out->type(type_e);
		out->typeval = type;
		return out;
	}
	static ehretval_p make(ehretval_p in) {
		return in;
	}
} ehretval_t;
typedef ehretval_t::ehretval_p ehretval_p;

// Operator
typedef struct opnode_t {
	int op; // Type of operator
	int nparas; // Number of parameters
	ehretval_p *paras; // Parameters
} opnode_t;

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
typedef ehmember_t *ehmember_p;

// in future, add type for type checking
typedef struct eharg_t {
	const char *name;
} eharg_t;

// context
typedef struct ehobj_t *ehcontext_t;

// library functions, classes, etcetera
typedef ehretval_p (*ehlibfunc_t)(int, ehretval_p *, ehcontext_t, class EHI *);

typedef void *(*ehconstructor_t)();

typedef ehretval_p (*ehlibmethod_t)(void *, int, ehretval_p *, ehcontext_t, class EHI *);

typedef struct ehlm_listentry_t {
	const char *name;
	ehlibmethod_t func;
} ehlm_listentry_t;

typedef struct ehlibclass_t {
	ehconstructor_t constructor;
	ehlm_listentry_t *members;
} ehlibclass_t;

// function executing a command
typedef ehretval_p (*ehcmd_t)(eharray_t *paras);

// EH array
typedef struct eharray_t {
	// typedefs
	typedef std::map<int, ehretval_p > int_map;
	typedef std::map<std::string, ehretval_p > string_map;
	typedef std::pair<const int, ehretval_p >& int_pair;
	typedef std::pair<const std::string, ehretval_p >& string_pair;
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
			case int_e: return this->int_indices.count(index->intval);
			case string_e: return this->string_indices.count(index->stringval);
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
	union {
		ehlibfunc_t libfunc_pointer;
		ehlibmethod_t libmethod_pointer;
	};
	
	ehfm_t(functype_enum _type) : type(_type), argcount(0), args(NULL), code(), libfunc_pointer(NULL) {}
	ehfm_t() : type(user_e), argcount(0), args(NULL), code(), libfunc_pointer(NULL) {}
	
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
	// properties
	ehfm_p function;
	const char *classname;
	struct ehobj_t *parent;
	struct ehobj_t *real_parent;
	// for instantiated and non-instantiated library classes
	void *selfptr;
	ehconstructor_t constructor;
	std::map<std::string, ehmember_p > members;

	// typedefs
	typedef std::map<std::string, ehmember_p > obj_map;
	typedef obj_map::iterator obj_iterator;
	
	// constructors
	ehobj_t(const char *_classname = NULL, ehobj_t *_parent = NULL, ehobj_t *_real_parent = NULL) : function(), classname(_classname), parent(_parent), real_parent(_real_parent), selfptr(NULL), constructor(NULL), members() {}

	// methods
	size_t size() const {
		return members.size();
	}
	
	ehmember_p insert_retval(const char *name, attributes_t attribute, ehretval_p value);
	ehmember_p get_recursive(const char *name, const ehcontext_t context, int token);
	ehmember_p get(const char *name, const ehcontext_t context, int token);
	void copy_member(obj_iterator &classmember, bool set_real_parent);

	ehmember_p &operator[](std::string key) {
		return members[key];
	}
	
	bool has(const std::string key) const {
		return members.count(key);
	}
	
	void insert(std::string &name, ehmember_p value) {
		members[name] = value;
	}
	void insert(const char *name, ehmember_p value) {
		std::string str(name);
		this->insert(str, value);
	}
	
	~ehobj_t() {
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
/*
 * Other global functions
 */
const char *get_typestring(type_enum type);
int eh_outer_exit(int exitval);
void yyerror(void *, const char *s);
char *eh_getinput(void);

/*
 * EH interpreter
 */
#include "eh.bison.hpp"
#include "ehi.h"

ehretval_p int_arrow_get(ehretval_p operand1, ehretval_p operand2);
ehretval_p string_arrow_get(ehretval_p operand1, ehretval_p operand2);
ehretval_p range_arrow_get(ehretval_p operand1, ehretval_p operand2);
void int_arrow_set(ehretval_p input, ehretval_p index, ehretval_p rvalue);
void string_arrow_set(ehretval_p input, ehretval_p index, ehretval_p rvalue);
void range_arrow_set(ehretval_p input, ehretval_p index, ehretval_p rvalue);
ehretval_p eh_count(const ehretval_p in);
ehretval_p eh_op_tilde(ehretval_p in);
ehretval_p eh_op_uminus(ehretval_p in);
ehretval_p eh_op_dot(ehretval_p operand1, ehretval_p operand2);
ehretval_p eh_make_range(const int min, const int max);
bool ehcontext_compare(const ehcontext_t lock, const ehcontext_t key);

// type casting
ehretval_p eh_cast(const type_enum type, ehretval_p in);
ehretval_p eh_stringtoint(const char *const in);
ehretval_p eh_stringtofloat(const char *const in);
ehretval_p eh_stringtorange(const char *const in);
ehretval_p eh_rangetoarray(const ehrange_t *const range);
char *eh_inttostring(const int in);
ehretval_p eh_xtoarray(ehretval_p in);
ehretval_p eh_xtoint(ehretval_p in);
ehretval_p eh_xtofloat(ehretval_p in);
ehretval_p eh_xtostring(ehretval_p in);
bool eh_xtobool(ehretval_p in);
ehretval_p eh_xtorange(ehretval_p in);
ehretval_p eh_looseequals(ehretval_p operand1, ehretval_p operand2);
bool eh_strictequals(ehretval_p operand1, ehretval_p operand2);

#endif /* EH_H_ */
