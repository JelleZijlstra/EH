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
 * The EH AST
 */

// macros to avoid having to check for NULL all the time
#define EH_TYPE(ret) (((ret) == NULL) ? null_e : (ret)->type)

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
	reference_e, // for internal use with lvalues, and as a value for references
	creference_e, // constant references: can be dereferenced but not written to
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
typedef struct memberattribute_t {
	visibility_enum visibility : 2;
	static_enum isstatic : 1;
	const_enum isconst : 1;
} memberattribute_t;

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
// Operator
typedef struct opnode_t {
	int op; // Type of operator
	int nparas; // Number of parameters
	struct ehretval_t **paras; // Parameters
} opnode_t;

// EH variable, and generic node
typedef struct ehretval_t {
	type_enum type;
	union {
		// simple EH variable type
		int intval;
		char *stringval;
		bool boolval;
		float floatval;
		// complex types
		struct eharray_t *arrayval;
		struct ehobj_t *objectval;
		struct ehretval_t *referenceval;
		struct ehobj_t *funcval;
		struct ehrange_t *rangeval;
		// pseudo-types for internal use
		opnode_t *opval;
		type_enum typeval;
		attribute_enum attributeval;
		memberattribute_t attributestrval;
		accessor_enum accessorval;
	};
	// constructors
	ehretval_t() {
		refcount = 1;
		is_shared = 0;
	}
	ehretval_t(type_enum mtype) {
		type = mtype;
		refcount = 1;
		is_shared = 0;
	}
#define EHRV_CONS(vtype, ehtype) ehretval_t(vtype in) { \
	type = ehtype ## _e; \
	ehtype ## val = in; \
	refcount = 1; \
	is_shared = 0; \
}
	EHRV_CONS(bool, bool)
	EHRV_CONS(double, float)
	EHRV_CONS(float, float)
	EHRV_CONS(char *, string)
	EHRV_CONS(int, int)
#undef EHRV_CONS

	// manipulate the refcount
	void inc_rc() {
		refcount++;
	}
	void dec_rc() {
		refcount--;
		if(refcount == 0) {
			// Commenting out the actual freeing until we actually keep track of 
			// refcounts correctly.
			//printf("Freeing ehretval_t at address %p\n", (void *)this);
			//free();
		}
	}
	void free();
	ehretval_t *clone() {
		ehretval_t *out = new ehretval_t;
		out->type = type;
		switch(type) {
#define COPY(type) case type ## _e: out->type ## val = type ## val; break
			COPY(int);
			COPY(string);
			COPY(bool);
			COPY(float);
			COPY(array);
			COPY(object);
			case creference_e:
			COPY(reference);
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
		return out;
	}
	// share this value with a new variable
	ehretval_t *share() {
		// can share if it's already shared or if there is only one reference
		if(is_shared || (refcount == 1)) {
			inc_rc();
			is_shared++;
			return this;
		} else {
			ehretval_t *out = clone();
			return out;
		}
	}
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
	}
	ehretval_t *overwrite(ehretval_t *in) {
		if(is_shared == 0) {
			// overwrite
			if(in == NULL) {
				type = null_e;
				return this;
			}
			type = in->type;
			switch(type) {
#define COPY(type) case type ## _e: type ## val = in->type ## val; break
				COPY(int);
				COPY(string);
				COPY(bool);
				COPY(float);
				COPY(array);
				COPY(object);
				case creference_e:
				COPY(reference);
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
			return this;
		} else {
			is_shared--;
			dec_rc();
			return in->reference(in);
		}
	}
	void make_shared() {
		is_shared++;
	}
private:
	short refcount;
	short is_shared;
} ehretval_t;

typedef struct ehmember_t {
	memberattribute_t attribute;
	struct ehretval_t *value;

	// destructor
	~ehmember_t() {
		// decrement refcount of the value
		if(value != NULL) {
			value->dec_rc();
		}
	}
	
	ehmember_t() {
		attribute.visibility = public_e;
		attribute.isstatic = nonstatic_e;
		attribute.isconst = nonconst_e;
	}
	ehmember_t(memberattribute_t atts) : attribute(atts) {}
} ehmember_t;

// in future, add type for type checking
typedef struct eharg_t {
	const char *name;
} eharg_t;

// context
typedef struct ehobj_t *ehcontext_t;

// library functions, classes, etcetera
typedef void (*ehlibfunc_t)(ehretval_t *, ehretval_t **, ehcontext_t, class EHI *);

typedef void *(*ehconstructor_t)();

typedef void (*ehlibmethod_t)(void *, ehretval_t *, ehretval_t **, ehcontext_t, class EHI *);

typedef struct ehlm_listentry_t {
	const char *name;
	ehlibmethod_t func;
} ehlm_listentry_t;

typedef struct ehlibclass_t {
	ehconstructor_t constructor;
	ehlm_listentry_t *members;
} ehlibclass_t;

// function executing a command
typedef ehretval_t *(*ehcmd_t)(eharray_t *paras);

// EH array
typedef struct eharray_t {
	typedef std::map<int, ehretval_t *> int_map;
	typedef std::map<std::string, ehretval_t *> string_map;
	typedef std::pair<const int, ehretval_t *>& int_pair;
	typedef std::pair<const std::string, ehretval_t *>& string_pair;
	typedef int_map::iterator int_iterator;
	typedef string_map::iterator string_iterator;

	int_map int_indices;
	string_map string_indices;

	ehretval_t * &operator[](ehretval_t *index);
	
	size_t size() {
		return this->int_indices.size() + this->string_indices.size();
	}
	
	bool has(ehretval_t *index) {
		switch(EH_TYPE(index)) {
			case int_e: return this->int_indices.count(index->intval);
			case string_e: return this->string_indices.count(index->stringval);
			default: return false;
		}
	}
	
	eharray_t() : int_indices(), string_indices() {}
} eharray_t;
#define ARRAY_FOR_EACH_STRING(array, varname) for(eharray_t::string_iterator varname = (array)->string_indices.begin(), end = (array)->string_indices.end(); varname != end; varname++)
#define ARRAY_FOR_EACH_INT(array, varname) for(eharray_t::int_iterator varname = (array)->int_indices.begin(), end = (array)->int_indices.end(); varname != end; varname++)

// EH object
typedef struct ehobj_t {
public:
	// properties
	struct ehfm_t *function;
	const char *classname;
	struct ehobj_t *parent;
	union {
		// for instantiated and non-instantiated library classes
		void *selfptr;
		ehconstructor_t constructor;
	};
	std::map<std::string, ehmember_t *> members;

	// typedefs
	typedef std::map<std::string, ehmember_t *> obj_map;
	typedef obj_map::iterator obj_iterator;
	
	// constructors
	ehobj_t() : function(NULL), classname(NULL), parent(NULL), members() {}

	// methods
	size_t size() {
		return members.size();
	}
	
	ehmember_t *insert_retval(const char *name, memberattribute_t attribute, ehretval_t *value);
	ehmember_t *get_recursive(const char *name, const ehcontext_t context, int token);
	ehmember_t *get(const char *name, const ehcontext_t context, int token);
	
	ehmember_t *&operator[](std::string key) {
		return members[key];
	}
	
	bool has(const std::string key) {
		return members.count(key);
	}
	
	void insert(std::string &name, ehmember_t *value) {
		members[name] = value;
	}
	void insert(const char *name, ehmember_t *value) {
		std::string str(name);
		this->insert(str, value);
	}
private:
	ehmember_t *get_recursive_helper(const char *name, const ehcontext_t context);
} ehobj_t;
#define OBJECT_FOR_EACH(obj, varname) for(ehobj_t::obj_iterator varname = (obj)->members.begin(), end = (obj)->members.end(); varname != end; varname++)

// range
typedef struct ehrange_t {
	int min;
	int max;
} ehrange_t;

// struct with common infrastructure for procedures and methods
typedef struct ehfm_t {
	functype_enum type;
	int argcount;
	eharg_t *args;
	union {
		ehretval_t *code;
		ehlibfunc_t libfunc_pointer;
		ehlibmethod_t libmethod_pointer;
	};
} ehfm_t;

// EH procedure

/*
 * EH error system
 */
typedef enum {
	efatal_e,
	eparsing_e,
	eerror_e, // runtime non-fatal error
	enotice_e
} errlevel_e;

void eh_error(const char *message, errlevel_e level);
void eh_error_type(const char *context, type_enum type, errlevel_e level);
void eh_error_looplevels(const char *context, int levels);
void eh_error_unknown(const char *kind, const char *name, errlevel_e level);
void eh_error_redefine(const char *kind, const char *name, errlevel_e level);
void eh_error_int(const char *message, int opcode, errlevel_e level);
void eh_error_argcount(int expected, int received);
void eh_error_line(int line, const char *msg);
void eh_error_types(const char *context, type_enum type1, type_enum type2, errlevel_e level);
void eh_error_argcount_lib(const char *name, int expected, int received);
/*
 * Other global functions
 */
const char *get_typestring(type_enum type);
int eh_outer_exit(int exitval);

/*
 * Top-level functions
 */
void yyerror(void *, const char *s);
void free_node(ehretval_t *in);
ehretval_t *eh_addnode(int operations, int noperations, ...);
void print_tree(const ehretval_t *const in, const int n);

// eh_get_x functions
#define GETFUNCPROTO(name, vtype) ehretval_t *eh_get_ ## name(vtype value);
ehretval_t *eh_get_null(void);
GETFUNCPROTO(int, int)
GETFUNCPROTO(string, char *)
GETFUNCPROTO(float, float)
GETFUNCPROTO(accessor, accessor_enum)
GETFUNCPROTO(type, type_enum)
GETFUNCPROTO(bool, bool)
GETFUNCPROTO(visibility, visibility_enum)
GETFUNCPROTO(attribute, attribute_enum)
#undef GETFUNCPROTO

char *eh_getinput(void);

/*
 * EH interpreter
 */
#include "eh.bison.hpp"
#include "ehi.h"

void class_copy_member(ehobj_t *classobj, ehobj_t::obj_iterator &classmember);
ehretval_t *int_arrow_get(ehretval_t *operand1, ehretval_t *operand2);
ehretval_t *string_arrow_get(ehretval_t *operand1, ehretval_t *operand2);
ehretval_t *range_arrow_get(ehretval_t *operand1, ehretval_t *operand2);
void int_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue);
void string_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue);
void range_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue);
ehretval_t *eh_count(const ehretval_t *in);
ehretval_t *eh_op_tilde(ehretval_t *in);
ehretval_t *eh_op_uminus(ehretval_t *in);
ehretval_t *eh_op_dot(ehretval_t *operand1, ehretval_t *operand2);
ehretval_t *eh_make_range(const int min, const int max);
void array_insert_retval(eharray_t *array, ehretval_t *index, ehretval_t *ret);
bool ehcontext_compare(const ehcontext_t lock, const ehcontext_t key);

// type casting
ehretval_t *eh_cast(const type_enum type, ehretval_t *in);
ehretval_t *eh_stringtoint(const char *const in);
ehretval_t *eh_stringtofloat(const char *const in);
ehretval_t *eh_stringtorange(const char *const in);
eharray_t *eh_rangetoarray(const ehrange_t *const range);
char *eh_inttostring(const int in);
ehretval_t *eh_xtoarray(ehretval_t *in);
ehretval_t *eh_xtoint(ehretval_t *in);
ehretval_t *eh_xtofloat(ehretval_t *in);
ehretval_t *eh_xtostring(ehretval_t *in);
bool eh_xtobool(ehretval_t *in);
ehretval_t *eh_xtorange(ehretval_t *in);
ehretval_t *eh_looseequals(ehretval_t *operand1, ehretval_t *operand2);
bool eh_strictequals(ehretval_t *operand1, ehretval_t *operand2);

/*
 * Helper
 */
int eh_getargs(ehretval_t *paras, int n, ehretval_t **args, ehcontext_t context, const char *name, EHI *obj);
void print_retval(const ehretval_t *in);

#endif /* EH_H_ */
