/*
 * eh_tree.c
 * Jelle Zijlstra, December 2011
 *
 * Functionality to create and show the EH abstract syntax tree.
 */
#include "eh.h"

void free_node(ehnode_t *in) {
	if(in == NULL)
		return;
	int i;
	switch(in->type) {
		case stringnode_e:
			free(in->stringv);
			break;
		case intnode_e:
		case nullnode_e:
		case boolnode_e:
		case visibilitynode_e:
		case accessornode_e:
		case magicvarnode_e:
		case typenode_e:
			// nothing to free
			break;
		case opnode_e:
			for(i = 0; i < in->op.nparas; i++) {
				free_node(in->op.paras[i]);
			}
			break;
	}
	free(in);
}

#define GETFUNC(name, vtype) ehnode_t *get_ ## name (vtype value) { \
	ehnode_t *ret; \
	ret = Malloc(sizeof(ehnode_t)); \
	ret->type = name ## node_e; \
	ret-> name ## v = value; \
	return ret; \
}
GETFUNC(int, int)
GETFUNC(string, char *)
ehnode_t *get_null(void) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));

	ret->type = nullnode_e;

	return ret;
}
GETFUNC(type, type_enum)
GETFUNC(accessor, accessor_enum)
GETFUNC(bool, bool)
GETFUNC(visibility, visibility_enum)
GETFUNC(magicvar, magicvar_enum)

ehnode_t *operate(int operation, int nparas, ...) {
	va_list args;
	ehnode_t *ret;
	int i;

	ret = Malloc(sizeof(ehnode_t));
	if(nparas)
		ret->op.paras = Malloc(nparas * sizeof(ehnode_t *));
	else
		ret->op.paras = NULL;

	ret->type = opnode_e;
	ret->op.op = operation;
	ret->op.nparas = nparas;
	//printf("Adding operation %d with %d paras\n", ret->op.op, ret->op.nparas);
	va_start(args, nparas);
	for(i = 0; i < nparas; i++) {
		ret->op.paras[i] = va_arg(args, ehnode_t *);
		//printf("Para %d is of type %d\n", i, ret->op.paras[i]->type);
	}
	va_end(args);

	return ret;
}

void *Malloc(size_t size) {
	void *ret;
	ret = malloc(size);
	if(ret == NULL) {
		yyerror("Unable to allocate memory");
		exit(1);
	}
	return ret;
}
void *Calloc(size_t count, size_t size) {
	void *ret;
	ret = calloc(count, size);
	if(ret == NULL) {
		yyerror("Unable to allocate memory");
		exit(1);
	}
	return ret;
}

static void printntabs(int n) {
	int i;
	for(i = 0; i < n; i++) {
		printf("\t");
	}
}
// print the abstract syntax tree
void print_tree(ehnode_t *in, int n) {
	int i;

	switch(in->type) {
		case stringnode_e:
			printntabs(n); printf("Type: stringnode\n");
			printntabs(n); printf("Value: %s\n", in->stringv);
			break;
		case intnode_e:
			printntabs(n); printf("Type: intnode\n");
			printntabs(n); printf("Value: %d\n", in->intv);
			break;
		case opnode_e:
			printntabs(n); printf("Type: opnode\n");
			printntabs(n); printf("Opcode: %d\n", in->op.op);
			printntabs(n); printf("Nparas: %d\n", in->op.nparas);
			for(i = 0; i < in->op.nparas; i++) {
				print_tree(in->op.paras[i], n + 1);
			}
			break;
		case nullnode_e:
			printntabs(n); printf("Type: nullnode\n");
			break;
		case accessornode_e:
			printntabs(n); printf("Type: accessornode\n");
			printntabs(n); printf("Value: %d\n", in->accessorv);
			break;
		case typenode_e:
			printntabs(n); printf("Type: typenode\n");
			printntabs(n); printf("Value: %d\n", in->typev);
			break;
		case boolnode_e:
			printntabs(n); printf("Type: boolnode\n");
			printntabs(n); printf("Value: %u\n", in->boolv);
			break;
		case visibilitynode_e:
			printntabs(n); printf("Type: visibilitynode\n");
			printntabs(n); printf("Value: %d\n", in->visibilityv);
			break;
		case magicvarnode_e:
			printntabs(n); printf("Type: magicvarnode\n");
			printntabs(n); printf("Value: %d\n", in->magicvarv);
			break;
	}
}

const char *get_typestring(type_enum type) {
	switch(type) {
		case int_e: return "int";
		case string_e: return "string";
		case bool_e: return "bool";
		case reference_e: return "reference";
		case null_e: return "null";
		case accessor_e: return "accessor";
		case type_e: return "type";
		case array_e: return "array";
		case func_e: return "function";
		case object_e: return "object";
		case magicvar_e: return "magicvar";
	}
	// to keep the compiler happy
	return "null";
}
