#include "eh.h"

void free_node(ehnode_t *in) {
	if(in == NULL)
		return;
	int i;
	switch(in->type) {
		case idnode_e:
			free(in->id.name);
			break;
		case connode_e:
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
ehnode_t *get_constant(int value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));

	ret->type = connode_e;
	ret->con.value = value;
	
	//printf("Returning constant %d\n", ret->con.value);
	return ret;
}
ehnode_t *get_identifier(char *value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));
	
	ret->type = idnode_e;
	ret->id.name = value;
	
	//printf("Returning identifier %s\n", ret->id.name);
	return ret;
}
ehnode_t *get_type(type_enum value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));
	
	ret->type = typenode_e;
	ret->typev = value;
	
	return ret;
}
ehnode_t *operate(int operation, int nparas, ...) {
	va_list args;
	ehnode_t *ret;
	int i;
	
	ret = Malloc(sizeof(ehnode_t));
	ret->op.paras = Malloc(nparas * sizeof(ehnode_t *));
	
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
}
void *Calloc(size_t count, size_t size) {
	void *ret;
	ret = calloc(count, size);
	if(ret == NULL) {
		yyerror("Unable to allocate memory");
		exit(1);
	}
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
		case idnode_e:
			printntabs(n); printf("Type: idnode\n");
			printntabs(n); printf("Value: %s\n", in->id.name);
			break;
		case connode_e:
			printntabs(n); printf("Type: connode\n");
			printntabs(n); printf("Value: %d\n", in->con.value);
			break;
		case opnode_e:
			printntabs(n); printf("Type: opnode\n");
			printntabs(n); printf("Opcode: %d\n", in->op.op);
			for(i = 0; i < in->op.nparas; i++) {
				print_tree(in->op.paras[i], n + 1);
			}
			break;
	}
}
