#include "eh.h"

void free_node(ehnode_t *in) {
	if(in == NULL)
		return;
	int i;
	switch(in->type) {
		case idnode_enum:
			free(in->id.name);
			break;
		case connode_enum:
			// nothing to free
			break;
		case opnode_enum:
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

	ret->type = connode_enum;
	ret->con.value = value;
	
	//printf("Returning constant %d\n", ret->con.value);
	return ret;
}
ehnode_t *get_identifier(char *value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));
	
	ret->type = idnode_enum;
	ret->id.name = value;
	
	//printf("Returning identifier %s\n", ret->id.name);
	return ret;
}
ehnode_t *operate(int operation, int nparas, ...) {
	va_list args;
	ehnode_t *ret;
	int i;
	
	ret = Malloc(sizeof(ehnode_t));
	ret->op.paras = Malloc(nparas * sizeof(ehnode_t *));
	
	ret->type = opnode_enum;
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
