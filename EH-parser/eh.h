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
#include "y.tab.h"

typedef enum {
	idnode_enum,
	connode_enum,
	opnode_enum
} node_enum;

typedef enum {
	string_enum,
	int_enum
} type_enum;

typedef enum {
	user_enum = 0,
	lib_enum = 1,
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
	};
} ehnode_t;

// EH variable
typedef struct ehvar_t {
	type_enum type;
	char *name;
	int scope;
	union {
		int intval;
		char *strval;
	};
	struct ehvar_t *next;
} ehvar_t;

typedef struct ehretval_t {
	type_enum type;
	union {
		int intval;
		char *strval;
	};
} ehretval_t;

// type checking
#define IS_INT(var) (var.type == int_enum)
#define IS_STRING(var) (var.type == string_enum)

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
void free_node(ehnode_t *in);
ehnode_t *get_constant(int value);
ehnode_t *get_identifier(char *value);
ehnode_t *operate(int operations, int noperations, ...);
ehretval_t execute(ehnode_t *node);
void print_tree(ehnode_t *in, int n);

#include "eh_libfuncs.h"
