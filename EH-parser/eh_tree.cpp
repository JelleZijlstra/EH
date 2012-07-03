/*
 * eh_tree.c
 * Jelle Zijlstra, December 2011
 *
 * Functionality to create and show the EH abstract syntax tree.
 */
#include "eh.h"

void free_node(ehretval_t *in) {
	if(in == NULL) {
		return;
	}
	switch(in->type) {
		case string_e:
			delete[] in->stringval;
			break;
		case op_e:
			for(int i = 0; i < in->opval->nparas; i++) {
				free_node(in->opval->paras[i]);
			}
			break;
		default:
			// nothing to free (even for the last few ones; even though they
			// always occupy malloc'ed memory, this function merely frees
			// memory allocated by the parser)
			break;
	}
	delete in;
}

#define GETFUNC(name, vtype) ehretval_t *eh_get_ ## name (vtype value) { \
	ehretval_t *ret = new ehretval_t; \
	ret->type = name ## _e; \
	ret->name ## val = value; \
	ret->inc_rc(); \
	return ret; \
}
GETFUNC(int, int)
GETFUNC(string, char *)
GETFUNC(float, float)
ehretval_t *eh_get_null(void) {
	ehretval_t *ret = new ehretval_t;

	ret->type = null_e;
	ret->inc_rc();

	return ret;
}
GETFUNC(type, type_enum)
GETFUNC(accessor, accessor_enum)
GETFUNC(bool, bool)
GETFUNC(attribute, attribute_enum)

ehretval_t *eh_addnode(int operation, int nparas, ...) {
	va_list args;
	ehretval_t *ret;

	ret = new ehretval_t;
	ret->opval = new opnode_t;
	if(nparas) {
		ret->opval->paras = new ehretval_t *[nparas];
	} else {
		ret->opval->paras = NULL;
	}

	ret->type = op_e;
	ret->inc_rc();
	ret->opval->op = operation;
	ret->opval->nparas = nparas;
	va_start(args, nparas);
	for(int i = 0; i < nparas; i++) {
		ret->opval->paras[i] = va_arg(args, ehretval_t *);
	}
	va_end(args);

	return ret;
}

static void printntabs(const int n) {
	for(int i = 0; i < n; i++) {
		printf("\t");
	}
}
// print the abstract syntax tree
#define PRINT_TREE_TYPE(vtype, format) case vtype ## _e: \
	printntabs(n); \
	printf("Value: %" #format "\n", in-> vtype ## val); \
	break;

void print_tree(const ehretval_t *const in, const int n) {
	printntabs(n); printf("Type: %s\n", get_typestring(in->type));
	switch(in->type) {
		PRINT_TREE_TYPE(string, s)
		PRINT_TREE_TYPE(int, d)
		case op_e:
			printntabs(n); printf("Opcode: %d\n", in->opval->op);
			printntabs(n); printf("Nparas: %d\n", in->opval->nparas);
			for(int i = 0; i < in->opval->nparas; i++) {
				print_tree(in->opval->paras[i], n + 1);
			}
			break;
		case null_e:
			break;
		PRINT_TREE_TYPE(accessor, d)
		PRINT_TREE_TYPE(type, d)
		PRINT_TREE_TYPE(attribute, d)
		case attributestr_e:
			printntabs(n);
			if(in->attributestrval.visibility == private_e)
				printf("private");
			else
				printf("public");
			if(in->attributestrval.isstatic == static_e)
				printf(", static");
			else
				printf(", nonstatic");
			if(in->attributestrval.isconst == const_e)
				printf(", const");
			else
				printf(", nonconst");
			printf("\n");
			break;
		PRINT_TREE_TYPE(bool, u)
		PRINT_TREE_TYPE(float, f)
		case range_e:
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
		case float_e: return "float";
		case reference_e: return "reference";
		case null_e: return "null";
		case accessor_e: return "accessor";
		case type_e: return "type";
		case array_e: return "array";
		case func_e: return "function";
		case object_e: return "object";
		case op_e: return "op";
		case attribute_e: return "attribute";
		case attributestr_e: return "attributestr";
		case range_e: return "range";
	}
	// to keep the compiler happy
	return "null";
}
