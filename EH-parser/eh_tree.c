/*
 * eh_tree.c
 * Jelle Zijlstra, December 2011
 *
 * Functionality to create and show the EH abstract syntax tree.
 */
#include "eh.h"

void free_node(ehretval_t *in) {
	if(in == NULL)
		return;
	int i;
	switch(in->type) {
		case string_e:
			free(in->stringval);
			break;
		case int_e:
		case null_e:
		case bool_e:
		case visibility_e:
		case accessor_e:
		case magicvar_e:
		case type_e:
		case array_e:
		case func_e:
		case reference_e:
		case object_e:
			// nothing to free (even for the last few ones; even though they
			// always occupy malloc'ed memory, this function merely frees
			// memory allocated by the parser)
			break;
		case op_e:
			for(i = 0; i < in->opval->nparas; i++) {
				free_node(in->opval->paras[i]);
			}
			break;
	}
	free(in);
}

#define GETFUNC(name, vtype) ehretval_t *get_ ## name (vtype value) { \
	ehretval_t *ret; \
	ret = Malloc(sizeof(ehretval_t)); \
	ret->type = name ## _e; \
	ret-> name ## val = value; \
	return ret; \
}
GETFUNC(int, int)
GETFUNC(string, char *)
ehretval_t *get_null(void) {
	ehretval_t *ret;
	ret = Malloc(sizeof(ehretval_t));

	ret->type = null_e;

	return ret;
}
GETFUNC(type, type_enum)
GETFUNC(accessor, accessor_enum)
GETFUNC(bool, bool)
GETFUNC(visibility, visibility_enum)
GETFUNC(magicvar, magicvar_enum)

ehretval_t *operate(int operation, int nparas, ...) {
	va_list args;
	ehretval_t *ret;
	int i;

	ret = Malloc(sizeof(ehretval_t));
	ret->opval = Malloc(sizeof(opnode_t));
	if(nparas)
		ret->opval->paras = Malloc(nparas * sizeof(ehretval_t *));
	else
		ret->opval->paras = NULL;

	ret->type = op_e;
	ret->opval->op = operation;
	ret->opval->nparas = nparas;
	//printf("Adding operation %d with %d paras\n", ret->opval->op, ret->opval->nparas);
	va_start(args, nparas);
	for(i = 0; i < nparas; i++) {
		ret->opval->paras[i] = va_arg(args, ehretval_t *);
		//printf("Para %d is of type %d\n", i, ret->opval->paras[i]->type);
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
void print_tree(ehretval_t *in, int n) {
	int i;
	printntabs(n); printf("Type: %s\n", get_typestring(in->type));
	switch(in->type) {
		case string_e:
			printntabs(n); printf("Value: %s\n", in->stringval);
			break;
		case int_e:
			printntabs(n); printf("Value: %d\n", in->intval);
			break;
		case op_e:
			printntabs(n); printf("Opcode: %d\n", in->opval->op);
			printntabs(n); printf("Nparas: %d\n", in->opval->nparas);
			for(i = 0; i < in->opval->nparas; i++) {
				print_tree(in->opval->paras[i], n + 1);
			}
			break;
		case null_e:
			break;
		case accessor_e:
			printntabs(n); printf("Value: %d\n", in->accessorval);
			break;
		case type_e:
			printntabs(n); printf("Value: %d\n", in->typeval);
			break;
		case bool_e:
			printntabs(n); printf("Value: %u\n", in->boolval);
			break;
		case visibility_e:
			printntabs(n); printf("Value: %d\n", in->visibilityval);
			break;
		case magicvar_e:
			printntabs(n); printf("Value: %d\n", in->magicvarval);
			break;
		case array_e:
		case func_e:
		case reference_e:
		case object_e:
			// don't appear in AST
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
		case op_e: return "op";
		case visibility_e: return "visibility";
	}
	// to keep the compiler happy
	return "null";
}
