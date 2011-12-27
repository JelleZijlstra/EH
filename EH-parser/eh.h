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
	nullnode_e,
	stringnode_e,
	intnode_e,
	opnode_e,
	typenode_e,
	boolnode_e
} node_enum;

typedef enum {
	null_e,
	string_e,
	int_e,
	array_e,
	type_e,
	bool_e
} type_enum;

typedef enum {
	user_e = 0,
	lib_e = 1,
} functype_enum;

// Identifier
typedef struct idnode_t {
	char *name;
} idnode_t;

// Constant
typedef struct connode_t {
	int value;
} connode_t;

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
		idnode_t id;
		connode_t con;
		opnode_t op;
		type_enum typev;
		bool boolv;
	};
} ehnode_t;

// EH variable
typedef struct ehvar_t {
	type_enum type;
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
	union {
		int intval;
		char *strval;
		struct ehvar_t **arrval;
		bool boolval;
	};
	struct ehvar_t *next;
} ehvar_t;

typedef struct ehretval_t {
	type_enum type;
	union {
		int intval;
		char *strval;
		ehvar_t **arrval;
		type_enum typeval;
		bool boolval;
	};
} ehretval_t;

// type checking
#define IS_INT(var) (var.type == int_e)
#define IS_STRING(var) (var.type == string_e)
#define IS_ARRAY(var) (var.type == array_e)
#define IS_BOOL(var) (var.type == bool_e)
#define IS_NULL(var) (var.type == null_e)

typedef struct ehfunc_t {
	char *name;
	functype_enum type: 1;
	int argcount: 31;
	ehnode_t **args;
	union {
		ehnode_t *code;
		void (*ptr)(ehnode_t *, ehretval_t *);
	};
	struct ehfunc_t *next;
} ehfunc_t;

typedef struct ehlibfunc_t {
	void (*code)(ehnode_t *, ehretval_t *);
	char *name;
} ehlibfunc_t;

void eh_init(void);
void eh_exit(void);

void yyerror(char *s);
void *Malloc(size_t size);
void *Calloc(size_t count, size_t size);
void free_node(ehnode_t *in);
ehnode_t *get_constant(int value);
ehnode_t *get_identifier(char *value);
ehnode_t *get_null(void);
ehnode_t *get_type(type_enum value);
ehnode_t *get_bool(bool value);
ehnode_t *operate(int operations, int noperations, ...);
ehretval_t execute(ehnode_t *node);
void print_tree(ehnode_t *in, int n);

void eh_setarg(int argc, char **argv);

#include "eh_libfuncs.h"
