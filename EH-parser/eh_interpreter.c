/*
 * eh_interpreter.c
 * Jelle Zijlstra, December 2011
 *
 * Implements a Lex- and Yacc-based interpreter for the EH scripting language.
 * Currently, does not yet support non-integer variables or library function
 * calls.
 */
#include "eh.h"
// symbol table for variables and functions
#define VARTABLE_S 1024
static ehvar_t *vartable[VARTABLE_S];
static ehfunc_t *functable[VARTABLE_S];

// indicate that we're returning
static bool returning = false;
// current variable scope
static int scope = 0;

// prototypes
static bool insert_variable(ehvar_t *var);
static ehvar_t *get_variable(char *name, int scope);
static void remove_variable(char *name, int scope);
static void list_variables(void);
static bool insert_function(ehfunc_t *func);
static ehfunc_t *get_function(char *name);
static void push_stack(ehnode_t *in);
static unsigned int hash(char *data);

ehlibfunc_t libfuncs[] = {
	{getinput, "getinput"},
	{NULL, NULL}
};

void eh_init(void) {
	int i;
	ehfunc_t *func;

	for(i = 0; libfuncs[i].code != NULL; i++) {
		func = Malloc(sizeof(ehfunc_t));
		func->name = libfuncs[i].name;
		func->type = lib_enum;
		func->ptr = libfuncs[i].code;
		// other fields are irrelevant
		insert_function(func);
	}
	return;
}
void eh_exit(void) {
	return;
}

int execute(ehnode_t *node) {
	// variable used
	ehvar_t *var;
	ehfunc_t *func;
	int ret, i, count;
	char *name;

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
					if(execute(node->op.paras[0])) {
						ret = execute(node->op.paras[1]);
						if(returning)
							return ret;
					}
					else if(node->op.nparas == 3)
						ret = execute(node->op.paras[2]);
					return ret;
				case T_WHILE:
					while(execute(node->op.paras[0])) {
						ret = execute(node->op.paras[1]);
						if(returning)
							return ret;
					}
					return ret;
				case T_FOR:
					// get the count
					count = execute(node->op.paras[0]);
					if(node->op.nparas == 2) {
						// "for 5; do stuff; endfor" construct
						for(i = 0; i < count; i++) {
							ret = execute(node->op.paras[1]);
							if(returning)
								return ret;
						}
					}
					else {
						// "for 5 count i; do stuff; endfor" construct
						name = node->op.paras[1]->id.name;
						var = get_variable(name, scope);
						// variable is not yet set, so set it
						if(var == NULL) {
							var = Malloc(sizeof(ehvar_t));
							var->name = node->op.paras[1]->id.name;
							var->scope = scope;
							insert_variable(var);
						}
						var->type = int_enum;
						for(var->intval = 0; var->intval < count; var->intval++) {
							ret = execute(node->op.paras[2]);
							if(returning)
								return ret;						
						}
					}
					return ret;
				case T_SEPARATOR:
					ret = execute(node->op.paras[0]);
					if(returning)
						return ret;
					ret = execute(node->op.paras[1]);
					return ret;
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
					name = node->op.paras[0]->id.name;
					var = get_variable(name, scope);
					if(var == NULL) {
						var = Malloc(sizeof(ehvar_t));
						var->name = name;
						// only supporting integer variables at present
						var->type = int_enum;
						var->scope = scope;
						insert_variable(var);
					}
					var->intval = execute(node->op.paras[1]);
					return 0;
				case '$': // variable dereference
					name = node->op.paras[0]->id.name;
					var = get_variable(name, scope);
					if(var == NULL) {
						fprintf(stderr, "Unknown variable %s\n", name);
						return 0;
					}
					return var->intval;
				case T_CALL: // call: execute argument and discard it
					execute(node->op.paras[0]);
					return 0;
				case ':': // function call
					name = node->op.paras[0]->id.name;
					func = get_function(name);
					//printf("Calling function %s at scope %d\n", node->op.paras[0]->id.name, scope);
					if(func == NULL) {
						fprintf(stderr, "Unknown function %s\n", name);
						return 0;						
					}
					if(func->type == lib_enum) {
						// library function
						func->ptr(node->op.paras[1], &ret);
						return ret;
					}
					int i = 0;
					ehnode_t *in = node->op.paras[1];
					while(1) {
						var = Malloc(sizeof(ehvar_t));
						var->name = func->args[i]->id.name;
						var->type = int_enum;
						var->scope = scope + 1;
						insert_variable(var);
						i++;
						if(i > func->argcount) {
							fprintf(stderr, "Incorrect argument count for function %s: expected %d, got %d\n", func->name, func->argcount, i);
							return 0;
						}
						if(in->type == opnode_enum && in->op.op == ',') {
							var->intval = execute(in->op.paras[0]);
							in = in->op.paras[1];
						}
						else {
							var->intval = execute(in);
							break;
						}
					}
					// functions get their own scope (not decremented before because execution of arguments needs parent scope)
					scope++;
					if(func->argcount != i) {
						fprintf(stderr, "Incorrect argument count for function %s: expected %d, got %d\n", name, func->argcount, i);
						return 0;
					}
					ret = execute(func->code);
					returning = false;
					for(i = 0; i < func->argcount; i++) {
						remove_variable(func->args[i]->id.name, scope);
					}
					scope--;
					return ret;
				case T_RET:
					returning = true;
					return execute(node->op.paras[0]);
				case T_FUNC: // function definition
					name = node->op.paras[0]->id.name;
					//printf("Defining function %s with %d paras\n", node->op.paras[0]->id.name, node->op.nparas);
					func = get_function(name);
					// function definition
					if(func != NULL) {
						fprintf(stderr, "Attempt to redefine function %s\n", name);
						return 0;
					}
					func = Malloc(sizeof(ehfunc_t));
					func->name = name;
					// determine argcount
					if(node->op.nparas == 2) {
						func->argcount = 0;
						func->args = NULL;
						func->code = node->op.paras[1];
					}
					else {
						func->argcount = 0;
						// traverse linked list to determine argument count
						ehnode_t *tmp;
						int currarg = 0;

						tmp = node->op.paras[1];
						while(1) {
							if(tmp->type == opnode_enum && tmp->op.op == ',') {
								currarg++;
								tmp = tmp->op.paras[1];
							}
							else {
								currarg++;
								break;
							}
						}
						func->argcount = currarg;
						func->args = Malloc(currarg * sizeof(char *));
						// add arguments to arglist
						tmp = node->op.paras[1];
						currarg = 0;
						while(1) {
							if(tmp->type == opnode_enum && tmp->op.op == ',') {
								func->args[currarg] = tmp->op.paras[0];
								currarg++;
								tmp = tmp->op.paras[1];
							}
							else {
								func->args[currarg] = tmp;
								break;
							}								
						}
						int j;
						func->code = node->op.paras[2];
					}
					func->type = user_enum;
					insert_function(func);
					return 0;
				default:
					fprintf(stderr, "Unexpected opcode %d\n", node->op.op);
					exit(0);
			}
	}
	return 0;
}

/*
 * Variables
 */
static bool insert_variable(ehvar_t *var) {
	unsigned int vhash;
	//printf("Inserting variable %s with value %d at scope %d\n", var->name, var->intval, var->scope);
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
static ehvar_t *get_variable(char *name, int scope) {
	unsigned int vhash;
	ehvar_t *currvar;
	
	vhash = hash(name);
	currvar = vartable[vhash];
	while(currvar != NULL) {
		//printf("name: %x, currvar->name, %x\n", name, currvar->name);
		if(strcmp(currvar->name, name) == 0 && currvar->scope == scope) {
			return currvar;
		}
		currvar = currvar->next;
	}
	return NULL;
}
static void remove_variable(char *name, int scope) {
	//printf("Removing variable %s of scope %d\n", name, scope);
	//list_variables();
	unsigned int vhash;
	ehvar_t *currvar;
	ehvar_t *prevvar;
	
	vhash = hash(name);
	currvar = vartable[vhash];
	prevvar = NULL;
	while(currvar != NULL) {
		if(strcmp(currvar->name, name) == 0 && currvar->scope == scope) {
			if(prevvar == NULL)
				vartable[vhash] = currvar->next;
			else
				prevvar->next = currvar->next;
			free(currvar);
			//list_variables();
			return;
		}
		prevvar = currvar;
		currvar = currvar->next;
	}
	return;
}
static void list_variables(void) {
	int i;
	ehvar_t *tmp;
	for(i = 0; i < VARTABLE_S; i++) {
		tmp = vartable[i];
		while(tmp != NULL) {
			printf("Variable %s of type %d at scope %d in hash %d at address %x\n", tmp->name, tmp->type, tmp->scope, i, tmp);
			tmp = tmp->next;
		}
	}
}
/*
 * Functions
 */
static bool insert_function(ehfunc_t *func) {
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
static ehfunc_t *get_function(char *name) {
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
