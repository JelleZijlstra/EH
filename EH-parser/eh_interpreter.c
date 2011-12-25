/*
 * eh_interpreter.c
 * Jelle Zijlstra, December 2011
 *
 * Implements a Lex- and Yacc-based interpreter for the EH scripting language.
 * Currently, does not yet support non-integer variables or library function
 * calls.
 */
#include "eh.h"
#include "y.tab.h"
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
static int eh_strtoi(char *in);
static char *eh_itostr(int in);

// macro for interpreter behavior
#define EH_INT_CASE(token, operator) case token: \
	operand1 = execute(node->op.paras[0]); \
	operand2 = execute(node->op.paras[1]); \
	if(IS_INT(operand1) && IS_INT(operand2)) { \
		ret.intval = (operand1.intval operator operand2.intval); \
	} \
	else if(IS_STRING(operand1) && IS_STRING(operand2)) { \
		ret.intval = (eh_strtoi(operand1.strval) operator eh_strtoi(operand2.strval)); \
	} \
	else if(IS_STRING(operand1)) { \
		i = eh_strtoi(operand1.strval); \
		ret.intval = (i operator operand2.intval); \
	} \
	else { \
		i = eh_strtoi(operand2.strval); \
		ret.intval = (i operator operand1.intval); \
	} \
	break;
#define SETRETFROMVAR(var) ret.type = var->type; switch(ret.type) { \
	case int_e: ret.intval = var->intval; break; \
	case string_e: ret.strval = var->strval; break; \
}
#define SETVARFROMRET(var) var->type = ret.type; switch(ret.type) { \
	case int_e: var->intval = ret.intval; break; \
	case string_e: var->strval = ret.strval; break; \
}

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
		func->type = lib_e;
		func->ptr = libfuncs[i].code;
		// other fields are irrelevant
		insert_function(func);
	}
	return;
}
void eh_exit(void) {
	return;
}

ehretval_t execute(ehnode_t *node) {
	// variable used
	ehvar_t *var;
	ehfunc_t *func;
	int i, count;
	char *name;
	ehretval_t ret, operand1, operand2;
	// default
	ret.type = int_e;
	ret.intval = 0;

	if(node == NULL)
		return ret;
	//printf("Executing nodetype %d\n", node->type);
	switch(node->type) {
		case idnode_e:
			ret.type = string_e;
			ret.strval = node->id.name;
			break;
		case connode_e:
			ret.intval = node->con.value;
			break;
		case typenode_e:
			ret.type = type_e;
			ret.typeval = node->typev;
			break;
		case opnode_e:
			//printf("Executing opcode: %d\n", node->op.op);
			switch(node->op.op) {
				case T_ECHO:
					switch(node->op.paras[0]->type) {
						case idnode_e:
							printf("%s\n", node->op.paras[0]->id.name);
							break;
						case connode_e:
							printf("%d\n", node->op.paras[0]->con.value);
							break;
						case opnode_e:
							ret = execute(node->op.paras[0]);
							switch(ret.type) {
								case string_e:
									printf("%s\n", ret.strval);
									break;
								case int_e:
									printf("%d\n", ret.intval);
									break;
							}
							break;
						default:
							fprintf(stderr, "Illegal argument for echo\n");
							break;
					}
					break;
				case T_IF:
					if(execute(node->op.paras[0]).intval) {
						ret = execute(node->op.paras[1]);
						if(returning)
							return ret;
					}
					else if(node->op.nparas == 3)
						ret = execute(node->op.paras[2]);
					break;
				case T_WHILE:
					while(execute(node->op.paras[0]).intval) {
						ret = execute(node->op.paras[1]);
						if(returning)
							return ret;
					}
					break;
				case T_FOR:
					// get the count
					count = execute(node->op.paras[0]).intval;
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
						// count variable always gets to be an int
						var->type = int_e;
						for(var->intval = 0; var->intval < count; var->intval++) {
							ret = execute(node->op.paras[2]);
							if(returning)
								return ret;						
						}
					}
					break;
				case T_SEPARATOR:
					ret = execute(node->op.paras[0]);
					if(returning)
						return ret;
					ret = execute(node->op.paras[1]);
					break;
				case '@': // type casting
					ret = execute(node->op.paras[0]);
					type_enum castto = ret.typeval;
					ret = execute(node->op.paras[1]);
					switch(castto) {
						case int_e:
							switch(ret.type) {
								case int_e:
									// nothing to cast
									break;
								case string_e:
									ret.intval = eh_strtoi(ret.strval);
									break;
							}
							ret.type = int_e;
							break;
						case string_e:
							switch(ret.type) {
								case int_e:
									ret.strval = eh_itostr(ret.intval);
									break;
								case string_e:
									// nothing to do
									break;
							}
							ret.type = string_e;
							break;
					}
					break;
				case '=':
					operand1 = execute(node->op.paras[0]);
					operand2 = execute(node->op.paras[1]);
					if(IS_INT(operand1) && IS_INT(operand2)) {
						ret.intval = (operand1.intval == operand2.intval);
					}
					else if(IS_STRING(operand1) && IS_STRING(operand2)) {
						ret.intval = !strcmp(operand1.strval, operand2.strval);
					}
					else if(IS_STRING(operand1)) {
						// type-juggle operand1
						i = eh_strtoi(operand1.strval);
						ret.intval = (i == operand2.intval);
					}
					else {
						i = eh_strtoi(operand2.strval);
						ret.intval = (i == operand1.intval);
					}
					break;
				EH_INT_CASE('>', <)
				EH_INT_CASE('<', <)
				EH_INT_CASE(T_GE, >=)
				EH_INT_CASE(T_LE, <=)
				EH_INT_CASE(T_NE, !=)
				// doing addition on two strings performs concatenation
				case '+':
					operand1 = execute(node->op.paras[0]);
					operand2 = execute(node->op.paras[1]);
					if(IS_INT(operand1) && IS_INT(operand2)) {
						ret.intval = (operand1.intval + operand2.intval);
					}
					else if(IS_STRING(operand1) && IS_STRING(operand2)) {
						// concatenate them
						ret.type = string_e;
						size_t len1, len2;
						len1 = strlen(operand1.strval);
						len2 = strlen(operand2.strval);
						ret.strval = Malloc(len1 + len2 + 1);
						strcpy(ret.strval, operand1.strval);
						strcpy(ret.strval + len1, operand2.strval);
					}
					else if(IS_STRING(operand1)) {
						// type-juggle operand1
						i = eh_strtoi(operand1.strval);
						ret.intval = (i + operand2.intval);
					}
					else {
						i = eh_strtoi(operand2.strval);
						ret.intval = (i + operand1.intval);
					}
					break;
				EH_INT_CASE('-', -)
				EH_INT_CASE('*', *)
				EH_INT_CASE('/', /)
				case T_SET:
					name = node->op.paras[0]->id.name;
					var = get_variable(name, scope);
					if(var == NULL) {
						var = Malloc(sizeof(ehvar_t));
						var->name = name;
						// only supporting integer variables at present
						var->scope = scope;
						insert_variable(var);
					}
					ret = execute(node->op.paras[1]);
					SETVARFROMRET(var);
					break;
				case '$': // variable dereference
					name = node->op.paras[0]->id.name;
					var = get_variable(name, scope);
					if(var == NULL) {
						fprintf(stderr, "Unknown variable %s\n", name);
						return ret;
					}
					SETRETFROMVAR(var);
					break;
				case T_CALL: // call: execute argument and discard it
					ret = execute(node->op.paras[0]);
					break;
				case ':': // function call
					name = node->op.paras[0]->id.name;
					func = get_function(name);
					//printf("Calling function %s at scope %d\n", node->op.paras[0]->id.name, scope);
					if(func == NULL) {
						fprintf(stderr, "Unknown function %s\n", name);
						return ret;						
					}
					if(func->type == lib_e) {
						// library function
						func->ptr(node->op.paras[1], &ret);
						return ret;
					}
					int i = 0;
					ehnode_t *in = node->op.paras[1];
					while(1) {
						var = Malloc(sizeof(ehvar_t));
						var->name = func->args[i]->id.name;
						var->scope = scope + 1;
						insert_variable(var);
						i++;
						if(i > func->argcount) {
							fprintf(stderr, "Incorrect argument count for function %s: expected %d, got %d\n", func->name, func->argcount, i);
							return ret;
						}
						if(in->type == opnode_e && in->op.op == ',') {
							ret = execute(in->op.paras[0]);
							SETVARFROMRET(var);
							in = in->op.paras[1];
						}
						else {
							ret = execute(in);
							SETVARFROMRET(var);
							break;
						}
					}
					// functions get their own scope (not decremented before because execution of arguments needs parent scope)
					scope++;
					if(func->argcount != i) {
						fprintf(stderr, "Incorrect argument count for function %s: expected %d, got %d\n", name, func->argcount, i);
						return ret;
					}
					ret = execute(func->code);
					returning = false;
					for(i = 0; i < func->argcount; i++) {
						remove_variable(func->args[i]->id.name, scope);
					}
					scope--;
					break;
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
						return ret;
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
							if(tmp->type == opnode_e && tmp->op.op == ',') {
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
							if(tmp->type == opnode_e && tmp->op.op == ',') {
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
					func->type = user_e;
					insert_function(func);
					break;
				default:
					fprintf(stderr, "Unexpected opcode %d\n", node->op.op);
					exit(0);
			}
	}
	return ret;
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

static int eh_strtoi(char *in) {
	int ret;
	ret = strtol(in, NULL, 0);
	if(ret == 0 && errno == EINVAL)
		fprintf(stderr, "Unable to perform type juggling\n");
	return ret;
}
static char *eh_itostr(int in) {
	char *buffer;
	int len;
	
	// INT_MAX has 10 decimal digits on this computer, so 12 (including sign and null terminator) should suffice for the result string
	buffer = Malloc(12);
	sprintf(buffer, "%d", in);
	
	return buffer;
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
