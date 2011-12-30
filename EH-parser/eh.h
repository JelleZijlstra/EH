/*
 * eh.h
 * Jelle Zijlstra, December 2011
 *
 * Main header file for the EH scripting language
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

typedef enum {
	this_e,
} magicvar_enum;

typedef enum {
	null_e = 0,
	string_e = 1,
	int_e = 2,
	bool_e = 3,
	accessor_e = 4, // for internal use with array/object accessors
	type_e = 5, // for internal use with type casting
	array_e,
	func_e, // for internal use with methods; might become a real user type in the future
	reference_e, // for internal use with lvalues, and as a value for references
	object_e,
	magicvar_e,
	op_e,
	attribute_e,
	attributestr_e,
} type_enum;

// attributes of class members
typedef enum {
	public_e = 0,
	private_e = 1,
} visibility_enum;

typedef enum {
	nonstatic_e = 0,
	static_e = 1,
} static_enum;

typedef enum {
	nonconst_e = 0,
	const_e = 1,
} const_enum;

typedef struct memberattribute_t {
	visibility_enum visibility : 2;
	static_enum isstatic : 1;
	const_enum isconst : 1;
} memberattribute_t;

typedef enum attribute_enum {
	publica_e,
	privatea_e,
	statica_e,
	consta_e
} attribute_enum;

typedef enum {
	user_e = 0,
	lib_e = 1,
} functype_enum;

typedef enum {
	arrow_e,
	dot_e,
	doublecolon_e
} accessor_enum;

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
		// complex types
		struct ehvar_t **arrayval;
		struct ehobj_t *objectval;
		struct ehretval_t *referenceval;
		struct ehfm_t *funcval;
		// pseudo-types for internal use
		opnode_t *opval;
		type_enum typeval;
		attribute_enum attributeval;
		memberattribute_t attributestrval;
		accessor_enum accessorval;
		magicvar_enum magicvarval;
	};
} ehretval_t;

typedef struct ehvar_t {
	// union: either the name of a variable or a numeric array index
	union {
		char *name;
		int index;
	};
	// either the scope of a variable or the index-type of an array member
	union {
		int scope;
		type_enum indextype;
	};
	struct ehretval_t value;
	struct ehvar_t *next;
} ehvar_t;

// type checking
#define IS_INT(var) (var.type == int_e)
#define IS_STRING(var) (var.type == string_e)
#define IS_ARRAY(var) (var.type == array_e)
#define IS_BOOL(var) (var.type == bool_e)
#define IS_NULL(var) (var.type == null_e)

// in future, add type for type checking
typedef struct eharg_t {
	char *name;
} eharg_t;

// EH object
typedef struct ehobj_t {
	char *class;
	struct ehclassmember_t **members;
} ehobj_t;

// context
typedef ehobj_t *ehcontext_t;

// class
typedef struct ehclass_t {
	ehobj_t obj;
	struct ehclass_t *next;
} ehclass_t;

// struct with common infrastructure for procedures and methods
typedef struct ehfm_t {
	functype_enum type;
	int argcount;
	eharg_t *args;
	union {
		ehretval_t *code;
		void (*ptr)(ehretval_t *, ehretval_t *, ehcontext_t);
	};
} ehfm_t;

// EH procedure
typedef struct ehfunc_t {
	char *name;
	struct ehfm_t f;
	struct ehfunc_t *next;
} ehfunc_t;

// Properties and methods of a class
typedef struct ehclassmember_t {
	memberattribute_t attribute;
	char *name;
	ehretval_t value;
	struct ehclassmember_t *next;
} ehclassmember_t;

typedef struct ehlibfunc_t {
	void (*code)(ehretval_t *, ehretval_t *, ehcontext_t);
	char *name;
} ehlibfunc_t;

void eh_init(void);
void eh_exit(void);
int yylex (void);

void yyerror(char *s);
void *Malloc(size_t size);
void *Calloc(size_t count, size_t size);
void free_node(ehretval_t *in);

#define GETFUNCPROTO(name, vtype) ehretval_t *get_ ## name(vtype value);
ehretval_t *get_constant(int value);
ehretval_t *get_identifier(char *value);
ehretval_t *get_null(void);
GETFUNCPROTO(int, int)
GETFUNCPROTO(string, char *)
GETFUNCPROTO(accessor, accessor_enum)
GETFUNCPROTO(type, type_enum)
GETFUNCPROTO(bool, bool)
GETFUNCPROTO(visibility, visibility_enum)
GETFUNCPROTO(magicvar, magicvar_enum)
GETFUNCPROTO(attribute, attribute_enum)

ehretval_t *operate(int operations, int noperations, ...);

ehretval_t execute(ehretval_t *node, ehcontext_t context);
void print_tree(ehretval_t *in, int n);
const char *get_typestring(type_enum type);

void eh_setarg(int argc, char **argv);

#include "eh_libfuncs.h"
