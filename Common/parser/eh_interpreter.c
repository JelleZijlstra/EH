#include "eh.h"
// symbol table for variables and functions
#define VARTABLE_S 1024
ehvar_t *vartable[VARTABLE_S];
ehfunc_t *functable[VARTABLE_S];

// argument stack for functions
#define STACKSIZE 64
ehnode_t *stack[STACKSIZE];
// stack pointer. esp != 0 also indicates that we want to execute a function
int esp = 0;

bool insert_variable(ehvar_t *var);
ehvar_t *get_variable(char *name);
bool insert_function(ehfunc_t *func);
ehfunc_t *get_function(char *name);
void push_stack(ehnode_t *in);
static unsigned int hash(char *data);

int execute(ehnode_t *node) {
	// variable used
	ehvar_t *var;
	ehfunc_t *func;

	if(node == NULL)
		return 0;
	//printf("Executing nodetype %d\n", node->type);
	switch(node->type) {
		case idnode_enum:
			return (int) node->id.name;
		case connode_enum:
			return node->con.value;
		case opnode_enum:
			//printf("Executing opcode: %d\n", node->op.op);
			switch(node->op.op) {
				case T_ECHO:
					switch(node->op.paras[0]->type) {
						case idnode_enum:
							printf("%s\n", node->op.paras[0]->id.name);
							return 0;
						case connode_enum:
							printf("%d\n", node->op.paras[0]->con.value);
							return 0;
						case opnode_enum:
							printf("%d\n", execute(node->op.paras[0]));
					}
					return 0;
				case T_IF:
					if(execute(node->op.paras[0]))
						execute(node->op.paras[1]);
					return 0;
				case T_WHILE:
					while(execute(node->op.paras[0]))
						execute(node->op.paras[1]);
					return 0;
				case T_SEPARATOR:
					execute(node->op.paras[0]);
					execute(node->op.paras[1]);
					return 0;
				case '=':
					return execute(node->op.paras[0]) == 
						execute(node->op.paras[1]);
				case '>':
					return execute(node->op.paras[0]) >
						execute(node->op.paras[1]);
				case '<':
					return execute(node->op.paras[0]) <
						execute(node->op.paras[1]);
				case T_GE:
					return execute(node->op.paras[0]) >= 
						execute(node->op.paras[1]);
				case T_LE:
					return execute(node->op.paras[0]) <=
						execute(node->op.paras[1]);
				case T_NE:
					return execute(node->op.paras[0]) != 
						execute(node->op.paras[1]);
				case '+':
					return execute(node->op.paras[0]) + 
						execute(node->op.paras[1]);
				case '-':
					return execute(node->op.paras[0]) - 
						execute(node->op.paras[1]);
				case '*':
					return execute(node->op.paras[0]) * 
						execute(node->op.paras[1]);
				case '/':
					return execute(node->op.paras[0]) / 
						execute(node->op.paras[1]);
				case T_SET:
					var = get_variable(node->op.paras[0]->id.name);
					if(var == NULL) {
						var = Malloc(sizeof(ehnode_t));
						var->name = node->op.paras[0]->id.name;
						// only supporting integer variables at present
						var->type = int_enum;
						insert_variable(var);
					}
					var->intval = execute(node->op.paras[1]);
					return 0;
				case '$': // variable dereference
					var = get_variable(node->op.paras[0]->id.name);
					if(var == NULL) {
						printf("Unknown variable %s\n", node->op.paras[0]->id.name);
						return 0;
					}
					return var->intval;
				case T_CALL: // call: execute argument and discard it
					execute(node->op.paras[0]);
					return 0;
				case ':': // function
					func = get_function(node->op.paras[0]->id.name);
					if(func == NULL) {
						printf("Unknown function %s\n", node->op.paras[0]->id.name);
						printf("%d\n", esp);
						return 0;						
					}
					if(node->op.paras[0] == NULL) {
						// no parameters
						esp = 1;
					}
					else
						push_stack(node->op.paras[1]);
					return execute(func->code);
				default:
					printf("Unexpected opcode %d\n", node->op.op);
					exit(0);
			}
	}
	return 0;
}

/*
 * Variables
 */
bool insert_variable(ehvar_t *var) {
	unsigned int vhash;
	
	vhash = hash(var->name);
	if(vartable[vhash] == NULL) {
		vartable[vhash] = var;
		var->next = NULL;
	}
	else {
		var->next = vartable[vhash];
		vartable[vhash] = var;
	}
	return true;
}
ehvar_t *get_variable(char *name) {
	unsigned int vhash;
	ehvar_t *currvar;
	
	vhash = hash(name);
	currvar = vartable[vhash];
	while(currvar != NULL) {
		if(strcmp(currvar->name, name) == 0) {
			return currvar;
		}
		currvar = currvar->next;
	}
	return NULL;
}

/*
 * Functions
 */
bool insert_function(ehfunc_t *func) {
	unsigned int vhash;
	
	vhash = hash(func->name);
	if(functable[vhash] == NULL) {
		functable[vhash] = func;
		func->next = NULL;
	}
	else {
		func->next = functable[vhash];
		functable[vhash] = func;
	}
	return true;
}
ehfunc_t *get_function(char *name) {
	unsigned int vhash;
	ehfunc_t *currfunc;

	vhash = hash(name);
	currfunc = functable[vhash];
	while(currfunc != NULL) {
		if(strcmp(currfunc->name, name) == 0) {
			return currfunc;
		}
		currfunc = currfunc->next;
	}
	return NULL;
}
void push_stack(ehnode_t *in) {
	esp++;
	if(in->type == opnode_enum && in->op.op == ',') {
		stack[esp] = in->op.paras[0];
		push_stack(in->op.paras[1]);
	}
	else
		stack[esp] = in;
}
/* Hash function */
// from http://azillionmonkeys.com/qed/hash.html
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

static unsigned int hash(char *data) {
    unsigned int hash, tmp, len;
    int rem;
    
    len = strlen(data);
    hash = len;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= data[sizeof (uint16_t)] << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += *data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash % VARTABLE_S;
}
