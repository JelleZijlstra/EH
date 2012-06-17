/*
 * eh.h
 * Jelle Zijlstra, December 2011
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

/*
 * The EH AST
 */

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
		struct ehvar_t **arrayval;
		struct ehobj_t *objectval;
		struct ehretval_t *referenceval;
		struct ehfm_t *funcval;
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
	ehretval_t *reference(ehretval_t **in) {
		if(is_shared == 0) {
			inc_rc();
			return this;
		} else {
			ehretval_t *out = clone();
			is_shared--;
			dec_rc();
			*in = out;
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
			return in->reference(&in);
		}
	}
	void make_shared() {
		is_shared++;
	}
private:
	short refcount;
	short is_shared;
} ehretval_t;

typedef struct ehvar_t {
	// union: either the name of a variable or a numeric array index
	union {
		const char *name;
		int index;
	};
	// the scope of a variable, the index-type of an array member, or the
	// attributes of an object member
	union {
		memberattribute_t attribute;
		unsigned long scope;
		type_enum indextype;
	};
	struct ehretval_t *value;
	struct ehvar_t *next;
	
	// destructor
	~ehvar_t() {
		// decrement refcount of the value
		if(value != NULL) {
			value->dec_rc();
		}
	}
} ehvar_t;

// in future, add type for type checking
typedef struct eharg_t {
	const char *name;
} eharg_t;

// context
typedef const struct ehobj_t *ehcontext_t;

typedef void *(*ehconstructor_t)();

typedef void (*ehlibmethod_t)(void *, ehretval_t *, ehretval_t **, ehcontext_t, class EHI *);

typedef struct ehlibentry_t {
	const char *name;
	ehlibmethod_t func;
} ehlibentry_t;

typedef struct ehlibclass_t {
	ehconstructor_t constructor;
	ehlibentry_t *members;
} ehlibclass_t;

typedef struct ehlc_listentry_t {
	const char *name;
	ehlibclass_t info;
} ehlc_listentry_t;

// EH object
typedef struct ehobj_t {
	const char *classname;
	union {
		void *selfptr;
		ehconstructor_t constructor;
	};
	struct ehvar_t **members;
} ehobj_t;

// range
typedef struct ehrange_t {
	int min;
	int max;
} ehrange_t;

// function executing a command
typedef ehretval_t *(*ehcmd_f_t)(ehvar_t **paras);

// command
typedef struct ehcmd_t {
	const char *name;
	ehcmd_f_t code;
} ehcmd_t;

typedef struct ehcmd_bucket_t {
	ehcmd_t cmd;
	struct ehcmd_bucket_t *next;
} ehcmd_bucket_t;

// scope
struct ehscope_t {
public:
	struct varscope_t {
		struct varscope_t *next;
		struct varscope_t *parent;
		varscope_t(varscope_t *in) {
			next = in;
		}
		varscope_t() {
			next = new varscope_t(NULL);
		}
	};
private:
	// current scope for this method
	varscope_t *var_scope;
public:
	struct ehscope_t *parent;
	// push and pop a new scope from the stack
	unsigned long push() {
		varscope_t *new_scope = new varscope_t(var_scope);
		new_scope->parent = parent->top_pointer();
		var_scope = new_scope;
		return (unsigned long) new_scope;
	}
	void pop() {
		assert(var_scope->next != NULL);
		varscope_t *tmp = var_scope->next;
		delete var_scope;
		var_scope = tmp;
	}
	// deferred push: create a new scope, but don't put it in yet
	unsigned long deferred_push() {
		varscope_t *new_scope = new varscope_t(var_scope);
		new_scope->parent = parent->top_pointer();
		return (unsigned long) new_scope;
	}
	void complete_push(unsigned long new_scope) {
		var_scope = (varscope_t *) new_scope;
	}
	unsigned long top() {
		return (unsigned long) var_scope;
	}
	varscope_t *top_pointer() {
		return var_scope;
	}
	// constructors
	ehscope_t(ehscope_t *n_parent) {
		parent = n_parent;
	}
	ehscope_t() {
		// used as constructor for the global_scope
		parent = NULL;
		var_scope = new varscope_t();
		var_scope->parent = NULL;
	}
};

// class
typedef struct ehclass_t {
	ehobj_t obj;
	functype_enum type;
	struct ehclass_t *next;
} ehclass_t;

// struct with common infrastructure for procedures and methods
typedef struct ehfm_t {
	functype_enum type;
	int argcount;
	eharg_t *args;
	ehscope_t scope;
	union {
		ehretval_t *code;
		void (*ptr)(ehretval_t *, ehretval_t **, ehcontext_t, class EHI *);
		ehlibmethod_t mptr;
	};
} ehfm_t;

// EH procedure
typedef struct ehlibfunc_t {
	void (*code)(ehretval_t *, ehretval_t **, ehcontext_t, class EHI *);
	const char *name;
} ehlibfunc_t;

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

ehretval_t *class_get(const ehobj_t *classobj, const char *name, ehcontext_t context);
void class_copy_member(ehobj_t *classobj, ehvar_t *classmember, int i);
ehvar_t *array_getmember(ehvar_t **array, ehretval_t *index);
ehvar_t *class_getmember(const ehobj_t *classobj, const char *name, ehcontext_t context);
void make_arglist(int *argcount, eharg_t **arglist, ehretval_t *node);
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
ehvar_t *class_insert_retval(ehvar_t **classarr, const char *name, memberattribute_t attribute, ehretval_t *value);
int array_count(ehvar_t **array);
ehvar_t *array_insert_retval(ehvar_t **array, ehretval_t *index, ehretval_t *ret);
bool ehcontext_compare(const ehcontext_t lock, const ehcontext_t key);
ehretval_t *array_get(ehvar_t **array, ehretval_t *index);


// generic initval for the hash function if no scope is applicable (i.e., for functions, which are not currently scoped)
#define HASH_INITVAL 234092
unsigned int hash(const char *data, int scope);

// type casting
ehretval_t *eh_cast(const type_enum type, ehretval_t *in);
ehretval_t *eh_stringtoint(const char *const in);
ehretval_t *eh_stringtofloat(const char *const in);
ehretval_t *eh_stringtorange(const char *const in);
ehvar_t **eh_rangetoarray(const ehrange_t *const range);
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

// macros to avoid having to check for NULL all the time
#define EH_TYPE(ret) (((ret) == NULL) ? null_e : (ret)->type)

#endif /* EH_H_ */
