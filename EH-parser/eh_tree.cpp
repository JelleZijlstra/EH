/*
 * eh_tree.c
 * Jelle Zijlstra, December 2011
 *
 * Functionality to create and show the EH abstract syntax tree.
 */
#include "eh.h"

#define GETFUNC(name, vtype) ehretval_p eh_get_ ## name (vtype value) { \
	ehretval_p ret; \
	ret->type(name ## _e); \
	ret->name ## val = value; \
	return ret; \
}
GETFUNC(int, int)
GETFUNC(string, char *)
GETFUNC(float, float)
ehretval_p eh_get_null(void) {
	return NULL;
}
GETFUNC(type, type_enum)
GETFUNC(accessor, accessor_enum)
GETFUNC(bool, bool)
GETFUNC(attribute, attribute_enum)
GETFUNC(op, opnode_t *)

opnode_t *eh_addnode_base(int opcode, int nparas, ehretval_p *paras) {
	opnode_t *ret = new opnode_t;
	ret->op = opcode;
	ret->nparas = nparas;
	ret->paras = paras;
	return ret;
}

opnode_t *eh_addnode(int opcode) {
	return eh_addnode_base(opcode, 0, NULL);
}
opnode_t *eh_addnode(int opcode, ehretval_p first) {
	ehretval_p *paras = new ehretval_p[1]();
	paras[0] = first;
	return eh_addnode_base(opcode, 1, paras);
}
opnode_t *eh_addnode(int opcode, ehretval_p first, ehretval_p second) {
	ehretval_p *paras = new ehretval_p[2]();
	paras[0] = first;
	paras[1] = second;
	return eh_addnode_base(opcode, 2, paras);
}
opnode_t *eh_addnode(int opcode, ehretval_p first, ehretval_p second, ehretval_p third) {
	ehretval_p *paras = new ehretval_p[3]();
	paras[0] = first;
	paras[1] = second;
	paras[2] = third;
	return eh_addnode_base(opcode, 3, paras);
}
opnode_t *eh_addnode(int opcode, ehretval_p first, ehretval_p second, ehretval_p third, ehretval_p fourth) {
	ehretval_p *paras = new ehretval_p[4]();
	paras[0] = first;
	paras[1] = second;
	paras[2] = third;
	paras[3] = fourth;
	return eh_addnode_base(opcode, 4, paras);
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

void print_tree(const ehretval_p in, const int n) {
	printntabs(n); printf("Type: %s\n", get_typestring(in->type()));
	switch(in->type()) {
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
