#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "y.tab.h"

typedef enum {
	idnode_enum,
	connode_enum,
	opnode_enum
} node_enum;

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

void yyerror(char *s);

void *Malloc(size_t size);
void free_node(ehnode_t *in);
ehnode_t *get_constant(int value);
ehnode_t *get_identifier(char *value);
ehnode_t *operate(int operations, int noperations, ...);
int execute(ehnode_t *node);