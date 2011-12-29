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
	nullnode_e = 0,
	stringnode_e = 1,
	intnode_e = 2,
	boolnode_e = 3,
	accessornode_e = 4,
	typenode_e = 5,
	opnode_e,
	visibilitynode_e,
	magicvarnode_e,
} node_enum;

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
} type_enum;

typedef enum {
	public_e,
	private_e
} visibility_enum;

typedef enum {
	user_e = 0,
	lib_e = 1,
} functype_enum;

typedef enum {
	arrow_e,
	dot_e,
} accessor_enum;

// Operator
typedef struct opnode_t {
	int op; // Type of operator
	int nparas; // Number of parameters
	struct ehnode_t **paras; // Parameters
} opnode_t;

// Generic node
typedef struct ehnode_t {
	node_enum type;
	union {
		char *stringv;
		int intv;
		opnode_t op;
		type_enum typev;
		visibility_enum visibilityv;
		bool boolv;
		accessor_enum accessorv;
		magicvar_enum magicvarv;
	};
} ehnode_t;

// EH variable
typedef struct ehretval_t {
	type_enum type;
	union {
		int intval;
		char *strval;
		struct ehvar_t **arrval;
		struct ehobj_t *objval;
		type_enum typeval;
		bool boolval;
		struct ehretval_t *referenceval;
		struct ehfm_t *funcval;
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
	struct ehclassmember_t **members;
	char *class;
} ehobj_t;

// context
typedef ehobj_t *ehcontext_t;

// struct with common infrastructure for procedures and methods
typedef struct ehfm_t {
	functype_enum type;
	int argcount;
	eharg_t *args;
	union {
		ehnode_t *code;
		void (*ptr)(ehnode_t *, ehretval_t *, ehcontext_t);
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
	visibility_enum visibility;
	char *name;
	ehretval_t value;
	struct ehclassmember_t *next;
} ehclassmember_t;

typedef struct ehclass_t {
	char *name;
	ehclassmember_t **members;
	struct ehclass_t *next;
} ehclass_t;

typedef struct ehlibfunc_t {
	void (*code)(ehnode_t *, ehretval_t *, ehcontext_t);
	char *name;
} ehlibfunc_t;

void eh_init(void);
void eh_exit(void);
int yylex (void);

void yyerror(char *s);
void *Malloc(size_t size);
void *Calloc(size_t count, size_t size);
void free_node(ehnode_t *in);

#define GETFUNCPROTO(name, vtype) ehnode_t *get_ ## name(vtype value);
ehnode_t *get_constant(int value);
ehnode_t *get_identifier(char *value);
ehnode_t *get_null(void);
GETFUNCPROTO(int, int)
GETFUNCPROTO(string, char *)
GETFUNCPROTO(accessor, accessor_enum)
GETFUNCPROTO(type, type_enum)
GETFUNCPROTO(bool, bool)
GETFUNCPROTO(visibility, visibility_enum)
GETFUNCPROTO(magicvar, magicvar_enum)

ehnode_t *operate(int operations, int noperations, ...);

ehretval_t execute(ehnode_t *node, ehcontext_t context);
void print_tree(ehnode_t *in, int n);
const char *get_typestring(type_enum type);

void eh_setarg(int argc, char **argv);

#include "eh_libfuncs.h"
