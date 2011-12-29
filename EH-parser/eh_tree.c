#include "eh.h"

void free_node(ehnode_t *in) {
	if(in == NULL)
		return;
	int i;
	switch(in->type) {
		case stringnode_e:
			free(in->id.name);
			break;
		case intnode_e:
		case nullnode_e:
		case boolnode_e:
		case visibilitynode_e:
		case accessornode_e:
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
ehnode_t *get_constant(int value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));

	ret->type = intnode_e;
	ret->con.value = value;

	//printf("Returning constant %d\n", ret->con.value);
	return ret;
}
ehnode_t *get_identifier(char *value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));

	ret->type = stringnode_e;
	ret->id.name = value;

	//printf("Returning identifier %s\n", ret->id.name);
	return ret;
}
ehnode_t *get_null(void) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));

	ret->type = nullnode_e;

	return ret;
}
ehnode_t *get_type(type_enum value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));

	ret->type = typenode_e;
	ret->typev = value;

	return ret;
}
ehnode_t *get_accessor(accessor_enum value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));
	
	ret->type = accessornode_e;
	ret->accessorv = value;
	
	return ret;
}
ehnode_t *get_bool(bool value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));
	
	ret->type = boolnode_e;
	ret->boolv = value;
	
	return ret;
}
ehnode_t *get_visibility(visibility_enum value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));

	ret->type = visibilitynode_e;
	ret->visibilityv = value;

	return ret;
}

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
			printntabs(n); printf("Value: %s\n", in->id.name);
			break;
		case intnode_e:
			printntabs(n); printf("Type: intnode\n");
			printntabs(n); printf("Value: %d\n", in->con.value);
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
	}
	// to keep the compiler happy
	return "null";
}
