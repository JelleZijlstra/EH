/*
 * eh_interpreter.c
 * Jelle Zijlstra, December 2011
 *
 * Implements a Lex- and Yacc-based interpreter for the EH scripting language.
 * Currently, does not yet support non-integer variables or library function
 * calls.
 */
#include "eh_interpreter.h"

// indicate that we're returning
static bool returning = false;
int scope = 0;
static void make_arglist(int *argcount, eharg_t **arglist, ehnode_t *node);

// library functions supported by ehi
ehlibfunc_t libfuncs[] = {
	{getinput, "getinput"},
	{printvar, "printvar"},
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
	// variables used
	ehvar_t *var, *member;
	ehfunc_t *func;
	ehclass_t *class;
	ehclassmember_t *classmember;
	int i, count;
	char *name;
	ehretval_t ret, operand1, operand2, operand3;
	// default
	ret.type = null_e;

	if(node == NULL)
		return ret;
	//printf("Executing nodetype %d\n", node->type);
	switch(node->type) {
		case stringnode_e:
			ret.type = string_e;
			ret.strval = node->id.name;
			break;
		case intnode_e:
			ret.type = int_e;
			ret.intval = node->con.value;
			break;
		case typenode_e:
			ret.type = type_e;
			ret.typeval = node->typev;
			break;
		case boolnode_e:
			ret.type = bool_e;
			ret.boolval = node->boolv;
			break;
		case nullnode_e:
			break;
		case opnode_e:
			//printf("Executing opcode: %d\n", node->op.op);
			switch(node->op.op) {
				case T_ECHO:
					switch(node->op.paras[0]->type) {
						case stringnode_e:
							printf("%s\n", node->op.paras[0]->id.name);
							break;
						case intnode_e:
							printf("%d\n", node->op.paras[0]->con.value);
							break;
						case nullnode_e:
							printf("(null)\n");
							break;
						case boolnode_e:
							if(node->op.paras[0]->boolv)
								printf("(true)\n");
							else
								printf("(false)\n");
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
								case bool_e:
									if(ret.boolval)
										printf("(true)\n");
									else
										printf("(false)\n");
									break;
								case null_e:
									printf("(null)\n");
									break;
								default:
									fprintf(stderr, "Cannot print this type\n");
							}
							break;
						default:
							fprintf(stderr, "Illegal argument for echo\n");
							break;
					}
					break;
				case T_IF:
					if(eh_xtobool(execute(node->op.paras[0])).boolval) {
						ret = execute(node->op.paras[1]);
						if(returning)
							return ret;
					}
					else if(node->op.nparas == 3)
						ret = execute(node->op.paras[2]);
					break;
				case T_WHILE:
					while(eh_xtobool(execute(node->op.paras[0])).boolval) {
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
						var->value.type = int_e;
						for(var->value.intval = 0; var->value.intval < count; var->value.intval++) {
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
				case T_ARROW: // array access, and similar stuff for other types
					operand1 = execute(node->op.paras[0]);
					operand2 = execute(node->op.paras[1]);
					switch(operand1.type) {
						case int_e:
							// "array" access to integer returns the nth bit of the integer; for example (assuming sizeof(int) == 32), (2 -> 30) == 1, (2 -> 31) == 0
							if(operand2.type != int_e) {
								fprintf(stderr, "Bitwise acess to an integer must use an integer identifier\n");
								return ret;
							}
							if(operand2.intval >= sizeof(int) * 8) {
								fprintf(stderr, "Identifier too large\n");
								return ret;
							}
							// get mask
							i = 1 << (sizeof(int) * 8 - 1);
							i >>= operand2.intval;
							// apply mask
							ret.intval = (operand1.intval & i) >> (sizeof(int) * 8 - 1 - i);
							ret.type = int_e;
							break;
						case string_e:
							// "array" access to a string returns an integer representing the nth character.
							// In the future, perhaps replace this with a char datatype or with a "shortstring" datatype representing strings up to 3 or even 4 characters long
							if(operand2.type != int_e) {
								fprintf(stderr, "Character acess to a string must use an integer identifier\n");
								return ret;
							}
							count = strlen(operand1.strval);
							if(operand2.intval >= count) {
								fprintf(stderr, "Identifier too large\n");
								return ret;
							}
							// get the nth character
							ret.intval = operand1.strval[operand2.intval];
							ret.type = int_e;
							break;
						case array_e:
							// array access to an array works as expected.
							ret = array_get(operand1.arrval, operand2);
							break;
						default:
							fprintf(stderr, "Array access from unsupported type\n");
							break;
					}
					break;
				case T_CLASS:
					class = Malloc(sizeof(ehclass_t));
					operand1 = execute(node->op.paras[0]);
					class->name = operand1.strval;
					class->members = Calloc(VARTABLE_S, sizeof(ehclassmember_t *));
					// insert class members
					node = node->op.paras[1];
					while(node != NULL) {
						if(node->type == opnode_e && node->op.op == ',') {
							class_insert(class->members, node->op.paras[0]);						
							node = node->op.paras[1];
						}
						else {
							class_insert(class->members, node);
							break;
						}
					}
					insert_class(class);
					break;
				case '[': // array declaration
					ret.type = array_e;
					ret.arrval = Calloc(VARTABLE_S, sizeof(ehvar_t *));
					i = 0;
					node = node->op.paras[0];
					while(1) {
						if(node->type == opnode_e && node->op.op == ',') {
							array_insert(ret.arrval, node->op.paras[0], i++);
							node = node->op.paras[1];
						}
						else {
							array_insert(ret.arrval, node, i++);
							break;
						}
					}
					break;
				case '@': // type casting
					operand1 = execute(node->op.paras[0]);
					operand2 = execute(node->op.paras[1]);
					switch(operand1.typeval) {
						case int_e:
							ret = eh_xtoi(operand2);
							break;
						case string_e:
							ret = eh_xtostr(operand2);
							break;
						case bool_e:
							ret = eh_xtobool(operand2);
							break;
						default:
							fprintf(stderr, "Unsupported typecast\n");
							break;
					}
					break;
				case T_EXPRESSION: // wrapper for special case
					return execute(node->op.paras[0]);
				case T_NEW:
					name = execute(node->op.paras[0]).strval;
					class = get_class(name);
					if(class == NULL) {
						fprintf(stderr, "No such class: %s\n", name);
						break;
					}
					ret.type = object_e;
					ret.objval = Calloc(VARTABLE_S, sizeof(ehclassmember_t *));
					ehclassmember_t *newmember;
					for(i = 0; i < VARTABLE_S; i++) {
						classmember = class->members[i];
						while(classmember != NULL) {
							newmember = Malloc(sizeof(ehclassmember_t));
							// copy the whole thing over
							*newmember = *classmember;
							newmember->next = ret.objval[i];
							ret.objval[i] = newmember;
							classmember = classmember->next;
						}
					}
					break;
				case '=':
					operand1 = execute(node->op.paras[0]);
					operand2 = execute(node->op.paras[1]);
					ret.type = bool_e;
					if(IS_INT(operand1) && IS_INT(operand2)) {
						ret.boolval = (operand1.intval == operand2.intval);
					}
					else if(IS_STRING(operand1) && IS_STRING(operand2)) {
						ret.boolval = !strcmp(operand1.strval, operand2.strval);
					}
					else {
						operand1 = eh_xtoi(operand1);
						operand2 = eh_xtoi(operand2);
						if(IS_INT(operand1) && IS_INT(operand2)) {
							ret.boolval = (operand1.intval == operand2.intval);
						}
						else
							ret.type = null_e;
					}
					break;
				case T_SE:
					operand1 = execute(node->op.paras[0]);
					operand2 = execute(node->op.paras[1]);
					ret.type = bool_e;
					if(IS_INT(operand1) && IS_INT(operand2)) {
						ret.boolval = (operand1.intval == operand2.intval);
					}
					else if(IS_STRING(operand1) && IS_STRING(operand2)) {
						ret.boolval = !strcmp(operand1.strval, operand2.strval);
					}
					else if(IS_BOOL(operand1) && IS_BOOL(operand2)) {
						ret.boolval = (operand1.boolval == operand2.boolval);
					}
					else if(IS_NULL(operand1) && IS_NULL(operand2)) {
						// null always equals null
						ret.boolval = true;
					}
					else {
						// strict comparison between different types always returns false
						ret.boolval = false;
						// TODO: array comparison
					}
					break;
				EH_INTBOOL_CASE('>', >)
				EH_INTBOOL_CASE('<', <)
				EH_INTBOOL_CASE(T_GE, >=)
				EH_INTBOOL_CASE(T_LE, <=)
				EH_INTBOOL_CASE(T_NE, !=)
				// doing addition on two strings performs concatenation
				case '+':
					operand1 = execute(node->op.paras[0]);
					operand2 = execute(node->op.paras[1]);
					if(IS_STRING(operand1) && IS_STRING(operand2)) {
						// concatenate them
						ret.type = string_e;
						size_t len1, len2;
						len1 = strlen(operand1.strval);
						len2 = strlen(operand2.strval);
						ret.strval = Malloc(len1 + len2 + 1);
						strcpy(ret.strval, operand1.strval);
						strcpy(ret.strval + len1, operand2.strval);
					}
					else {
						operand1 = eh_xtoi(operand1);
						operand2 = eh_xtoi(operand2);
						if(IS_INT(operand1) && IS_INT(operand2)) {
							ret.type = int_e;
							ret.intval = (operand1.intval + operand2.intval);
						}
					}
					break;
				EH_INT_CASE('-', -)
				EH_INT_CASE('*', *)
				EH_INT_CASE('/', /)
				case T_LVALUE:
					/*
					 * Get an lvalue. This case normally returns an
					 * ehretval_t of type retvalptr_e: a pointer to an
					 * ehretval_t that can be modified by the calling code.
					 *
					 * Because of special needs of calling code, this case
					 * actually returns useful data in the second field of the
					 * ehretval_t struct if its type is null_e. This is going to
					 * be a pointer to the ehretval_t of the variable referred 
					 * to, so that T_SET can do its bitwise magic with ints and 
					 * similar stuff. The second member can also have the 
					 * special values NULL (if referring to a non-existing 
					 * variable) and 0x1 (if referring to a member of a non-
					 * existing variable).
					 */
					name = node->op.paras[0]->id.name;
					var = get_variable(name, scope);
					switch(node->op.nparas) {
						case 1:
							if(var == NULL) {
								/*
								 * There is no variable of this name, and it is
								 * a simple access. In that case, we use NULL
								 * as the ptrval.
								 */
								ret.ptrval = NULL;
							}
							else {
								ret.type = retvalptr_e;
								ret.ptrval = &var->value;
							}
							break;
						case 2:
							if(var == NULL) {
								fprintf(stderr, "Cannot access member of non-existing variable\n");
								ret.ptrval = (ehretval_t *) 0x1;
							}
							else switch(var->value.type) {
								case array_e:
									operand1 = execute(node->op.paras[1]);							
									member = array_getmember(var->value.arrval, operand1);
									// if there is no member yet, insert it with a null value
									if(member == NULL) {
										member = array_insert_retval(var->value.arrval, operand1, ret);
									}
									ret.type = retvalptr_e;
									ret.ptrval = &member->value;
									break;
								default:
									ret.ptrval = &var->value;
									break;
							}
							break;
					}
					break;
				case T_SET:
					operand1 = execute(node->op.paras[0]);
					operand2 = execute(node->op.paras[1]);
					if(operand1.type == retvalptr_e) {
						// set variable
						*(operand1.ptrval) = operand2;
					}
					else if(operand1.type == null_e) {
						if(operand1.ptrval == NULL) {
							// set new variable
							var = Malloc(sizeof(ehvar_t));
							var->name = node->op.paras[0]->op.paras[0]->id.name;
							var->scope = scope;
							var->value = operand2;
							insert_variable(var);
						}
						else if(operand1.ptrval == (ehretval_t *) 0x1) {
							// do nothing; T_LVALUE will already have complained
						}
						else {
							// operand 1 is the variable modified, operand 2 is the value set to, operand 3 is the index
							operand3 = execute(node->op.paras[0]->op.paras[1]);
							switch(operand1.ptrval->type) {
								case int_e:
									if(operand3.type != int_e) {
										fprintf(stderr, "Bitwise acess to an integer must use an integer identifier\n");
										return ret;
									}
									if(operand3.intval >= sizeof(int) * 8) {
										fprintf(stderr, "Identifier too large\n");
										return ret;
									}
									// get mask
									i = (1 << (sizeof(int) * 8 - 1)) >> operand3.intval;
									if(eh_xtobool(operand2).boolval)
										operand1.ptrval->intval |= i;
									else {
										i = ~i;
										operand1.ptrval->intval &= i;
									}
									break;
								case string_e:
									if(operand3.type != int_e) {
										fprintf(stderr, "Character acess to a string must use an integer identifier\n");
										return ret;
									}
									if(operand2.type != int_e) {
										fprintf(stderr, "Character access to a string must use an integer to set\n");
										return ret;
									}
									count = strlen(operand1.ptrval->strval);
									if(operand3.intval >= count) {
										fprintf(stderr, "Identifier too large\n");
										return ret;
									}
									// get the nth character
									operand1.ptrval->strval[operand3.intval] = operand2.intval;
									break;
								default:
									fprintf(stderr, "Cannot set member of variable of this type\n");
									break;
							}
						}
					}
					break;
				case T_MINMIN:
					operand1 = execute(node->op.paras[0]);
					if(operand1.type == null_e) {
						fprintf(stderr, "Cannot set with -- operator\n");
						break;
					}
					switch(operand1.ptrval->type) {
						case int_e:
							operand1.ptrval->intval--;
							break;
						default:
							fprintf(stderr, "Unsupported type for -- operator\n");
							break;
					}
					break;
				case T_PLUSPLUS:
					operand1 = execute(node->op.paras[0]);
					if(operand1.type == null_e) {
						fprintf(stderr, "Cannot set with ++ operator\n");
						break;
					}
					switch(operand1.ptrval->type) {
						case int_e:
							operand1.ptrval->intval++;
							break;
						default:
							fprintf(stderr, "Unsupported type for ++ operator\n");
							break;
					}
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
					execute(node->op.paras[0]);
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
						if(node->op.nparas == 1)
							func->ptr(NULL, &ret);
						else
							func->ptr(node->op.paras[1], &ret);
						return ret;
					}
					int i = 0;
					// set parameters as necessary
					if(node->op.nparas == 2) {
						ehnode_t *in = node->op.paras[1];
						while(1) {
							var = Malloc(sizeof(ehvar_t));
							var->name = func->args[i].name;
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
					}
					// functions get their own scope (not decremented before because execution of arguments needs parent scope)
					scope++;
					if(func->argcount != i) {
						fprintf(stderr, "Incorrect argument count for function %s: expected %d, got %d\n", name, func->argcount, i);
						return ret;
					}
					ret = execute(func->code);
					returning = false;
					if(node->op.nparas == 2) {
						for(i = 0; i < func->argcount; i++) {
							remove_variable(func->args[i].name, scope);
						}
					}
					scope--;
					break;
				case T_RET:
					returning = true;
					return execute(node->op.paras[0]);
				case T_COUNT:
					operand1 = execute(node->op.paras[0]);
					ret.type = int_e;
					switch(operand1.type) {
						case int_e:
							ret.intval = sizeof(int) * 8;
							break;
						case string_e:
							ret.intval = strlen(operand1.strval);
							break;
						case array_e:
							ret.intval = array_count(operand1.arrval);
							break;
						case null_e:
							ret.intval = 0;
							break;
						case bool_e:
							ret.intval = 0;
							break;
						default:
							fprintf(stderr, "Unsupported datatype for count\n");
							break;
					}
					return ret;
				case '.': // object access
					operand1 = execute(node->op.paras[0]);
					if(operand1.type != object_e) {
						fprintf(stderr, "Access to variable that is not an object\n");
						break;
					}
					name = execute(node->op.paras[1]).strval;
					// TODO: make this support protected
					ret = class_get(operand1.objval, name);
					// method; nothing else to do for properties
					if(node->op.nparas == 3) {
						// split off a function from ':'?
					}
					break;
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
						make_arglist(&func->argcount, &func->args, node->op.paras[1]);
						func->code = node->op.paras[2];
					}
					func->type = user_e;
					insert_function(func);
					break;
				default:
					fprintf(stderr, "Unexpected opcode %d\n", node->op.op);
					exit(0);
			}
			break;
		default:
			fprintf(stderr, "Unsupported node type\n");
			break;
	}
	return ret;
}
/*
 * Variables
 */
bool insert_variable(ehvar_t *var) {
	unsigned int vhash;
	//printf("Inserting variable %s with value %d at scope %d\n", var->name, var->intval, var->scope);
	vhash = hash(var->name, var->scope);
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
ehvar_t *get_variable(char *name, int scope) {
	unsigned int vhash;
	ehvar_t *currvar;

	vhash = hash(name, scope);
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
void remove_variable(char *name, int scope) {
	//printf("Removing variable %s of scope %d\n", name, scope);
	//list_variables();
	unsigned int vhash;
	ehvar_t *currvar;
	ehvar_t *prevvar;

	vhash = hash(name, scope);
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
void list_variables(void) {
	int i;
	ehvar_t *tmp;
	for(i = 0; i < VARTABLE_S; i++) {
		tmp = vartable[i];
		while(tmp != NULL) {
			printf("Variable %s of type %d at scope %d in hash %d at address %x\n", tmp->name, tmp->value.type, tmp->scope, i, (int) tmp);
			tmp = tmp->next;
		}
	}
}
/*
 * Functions
 */
bool insert_function(ehfunc_t *func) {
	unsigned int vhash;

	vhash = hash(func->name, HASH_INITVAL);
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

	vhash = hash(name, HASH_INITVAL);
	currfunc = functable[vhash];
	while(currfunc != NULL) {
		if(strcmp(currfunc->name, name) == 0) {
			return currfunc;
		}
		currfunc = currfunc->next;
	}
	return NULL;
}
static void make_arglist(int *argcount, eharg_t **arglist, ehnode_t *node) {
	*argcount = 0;
	// traverse linked list to determine argument count
	ehnode_t *tmp;
	int currarg = 0;

	tmp = node;
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
	*argcount = currarg;
	*arglist = Malloc(currarg * sizeof(eharg_t));
	// add arguments to arglist
	tmp = node;
	currarg = 0;
	while(1) {
		if(tmp->type == opnode_e && tmp->op.op == ',') {
			(*arglist)[currarg].name = tmp->op.paras[0]->id.name;
			currarg++;
			tmp = tmp->op.paras[1];
		}
		else {
			(*arglist)[currarg].name = tmp->id.name;
			break;
		}
	}
}
/*
 * Classes
 */
void insert_class(ehclass_t *class) {
	unsigned int vhash;

	vhash = hash(class->name, HASH_INITVAL);
	if(classtable[vhash] == NULL) {
		classtable[vhash] = class;
		class->next = NULL;
	}
	else {
		class->next = classtable[vhash];
		classtable[vhash] = class;
	}
	return;
}
ehclass_t *get_class(char *name) {
	unsigned int vhash;
	ehclass_t *currclass;

	vhash = hash(name, HASH_INITVAL);
	currclass = classtable[vhash];
	while(currclass != NULL) {
		if(strcmp(currclass->name, name) == 0) {
			return currclass;
		}
		currclass = currclass->next;
	}
	return NULL;
}
void class_insert(ehclassmember_t **class, ehnode_t *in) {
	// insert a member into a class
	unsigned int vhash;
	ehclassmember_t *member;
	
	member = Malloc(sizeof(ehclassmember_t));
	// rely on standard layout of the input ehnode_t
	member->visibility = in->op.paras[0]->visibilityv;
	member->name = in->op.paras[1]->id.name;

	// decide what we got
	switch(in->op.nparas) {
		case 2: // non-set property: null
			member->value.type = null_e;
			break;
		case 3: // set property
			member->value = execute(in->op.paras[2]);
			break;
		case 4: // method
			member->value.type = func_e;
			member->value.funcval = in->op.paras[3];
			make_arglist(&member->argcount, &member->args, in->op.paras[2]);
			break;
	}

	// insert into hash table
	vhash = hash(member->name, 0);	
	member->next = class[vhash];
	class[vhash] = member;
}
ehclassmember_t *class_getmember(ehclassmember_t **class, char *name) {
	ehclassmember_t *curr;
	unsigned int vhash;
	
	vhash = hash(name, 0);
	curr = class[vhash];
	while(curr != NULL) {
		if(!strcmp(curr->name, name))
			break;
		curr = curr->next;
	}
	return curr;
}
ehretval_t class_get(ehclassmember_t **class, char *name) {
	ehclassmember_t *curr;
	ehretval_t ret;
	
	curr = class_getmember(class, name);
	if(curr == NULL)
		ret.type = null_e;
	else
		ret = curr->value;
	return ret;
}
/*
 * Type casting
 */
ehretval_t eh_strtoi(char *in) {
	ehretval_t ret;
	ret.type = int_e;
	ret.intval = strtol(in, NULL, 0);
	if(ret.intval == 0 && errno == EINVAL) {
		ret.type = null_e;
		fprintf(stderr, "Unable to perform type juggling\n");
	}
	return ret;
}
char *eh_itostr(int in) {
	char *buffer;

	// INT_MAX has 10 decimal digits on this computer, so 12 (including sign and null terminator) should suffice for the result string
	buffer = Malloc(12);
	sprintf(buffer, "%d", in);

	return buffer;
}
ehretval_t eh_xtoi(ehretval_t in) {
	ehretval_t ret;
	ret.type = int_e;
	switch(in.type) {
		case int_e:
			ret.intval = in.intval;
			break;
		case string_e:
			ret = eh_strtoi(in.strval);
			break;
		case bool_e:
			if(in.boolval)
				ret.intval = 1;
			else
				ret.intval = 0;
			break;
		case null_e:
			ret.intval = 0;
			break;
		default:
			fprintf(stderr, "Unsupported typecast to integer\n");
			ret.type = null_e;
			break;
	}
	return ret;
}
ehretval_t eh_xtostr(ehretval_t in) {
	ehretval_t ret;
	ret.type = string_e;
	switch(in.type) {
		case string_e:
			ret.strval = in.strval;
			break;
		case int_e:
			ret.strval = eh_itostr(in.intval);
			break;
		case null_e:
			// empty string
			ret.strval = Malloc(1);
			ret.strval[0] = '\0';
			break;
		case bool_e:
			if(in.boolval) {
				ret.strval = Malloc(5);
				strcpy(ret.strval, "true");
			}
			else {
				ret.strval = Malloc(6);
				strcpy(ret.strval, "false");
			}
			break;
		default:
			fprintf(stderr, "Unsupported typecast to string\n");
			ret.type = null_e;
			break;
	}
	return ret;
}
ehretval_t eh_xtobool(ehretval_t in) {
	ehretval_t ret;
	ret.type = bool_e;
	// convert an arbitrary variable to a bool
	switch(in.type) {
		case int_e:
			if(in.intval == 0)
				ret.boolval = false;
			else
				ret.boolval = true;
			break;
		case string_e:
			if(strlen(in.strval) == 0)
				ret.boolval = false;
			else
				ret.boolval = true;
			break;
		case bool_e:
			ret.boolval = in.boolval;
			break;
		case array_e:
			// ultimately, empty arrays should return false
			ret.boolval = true;
			break;
		default:
			// other types are always false
			ret.boolval = false;
			break;
	}
	return ret;
}
/*
 * Arrays
 */
void array_insert(ehvar_t **array, ehnode_t *in, int place) {
	unsigned int vhash;
	ehretval_t var;
	ehretval_t label;

	// new array member
	ehvar_t *member = Malloc(sizeof(ehvar_t));

	/*
	 * We'll assume we're always getting a correct ehnode_t *, referring to a
	 * T_ARRAYMEMBER token. If there is 1 parameter, that means it's a
	 * non-labeled array member, which we'll give an integer array index; if
	 * there are 2, we'll either use the integer array index or a hash of the
	 * string index.
	 */
	if(in->op.nparas == 1) {
		// if there is no explicit key, simply use the place argument
		vhash = place % VARTABLE_S;
		var = execute(in->op.paras[0]);
		member->indextype = int_e;
		member->index = place;
	}
	else {
		label = execute(in->op.paras[0]);
		switch(label.type) {
			case int_e:
				vhash = label.intval % VARTABLE_S;
				member->indextype = int_e;
				member->index = label.intval;
				break;
			case string_e:
				vhash = hash(label.strval, 0);
				member->indextype = string_e;
				member->name = label.strval;
				break;
			default:
				fprintf(stderr, "Unsupported label type\n");
				free(member);
				return;
		}
		var = execute(in->op.paras[1]);
	}

	// create array member
	member->value = var;

	// insert it into the hashtable
	ehvar_t **currptr = &array[vhash];
	switch(member->indextype) {
		case int_e:
			while(*currptr != NULL) {
				if((*currptr)->indextype == int_e && (*currptr)->index == member->index) {
					// replace this array member. I suppose this will break if we somehow enable references.
					member->next = (*currptr)->next;
					free(*currptr);
					*currptr = member;
					return;
				}
				currptr = &(*currptr)->next;
			}
			break;
		case string_e:
			while(*currptr != NULL) {
				if((*currptr)->indextype == string_e && !strcmp((*currptr)->name, member->name)) {
					member->next = (*currptr)->next;
					free(*currptr);
					*currptr = member;
					return;
				}
				currptr = &(*currptr)->next;
			}
			break;
		default:
			fprintf(stderr, "Unsupported index type\n");
			break;
	}
	*currptr = member;
	return;
}
ehvar_t *array_insert_retval(ehvar_t **array, ehretval_t index, ehretval_t ret) {
	// Inserts a member into an array. Assumes that the member is not yet present in the array.
	ehvar_t *new;
	unsigned int vhash;

	new = Malloc(sizeof(ehvar_t));
	new->indextype = index.type;
	switch(index.type) {
		case int_e:
			vhash = index.intval % VARTABLE_S;
			new->index = index.intval;
			break;
		case string_e:
			vhash = hash(index.strval, 0);
			new->name = index.strval;
			break;
		default:
			fprintf(stderr, "Unsupported array index type\n");
			break;
	}
	new->next = array[vhash];
	array[vhash] = new;
	SETVARFROMRET(new);
	return new;
}
ehvar_t *array_getmember(ehvar_t **array, ehretval_t index) {
	ehvar_t *curr;
	unsigned int vhash;

	switch(index.type) {
		case int_e:
			vhash = index.intval % VARTABLE_S;
			break;
		case string_e:
			vhash = hash(index.strval, 0);
			break;
		default:
			fprintf(stderr, "Unsupported array index type\n");
			return NULL;
	}
	curr = array[vhash];
	switch(index.type) {
		case int_e:
			while(curr != NULL) {
				if(curr->indextype == int_e && curr->index == index.intval)
					return curr;
				curr = curr->next;
			}
			break;
		case string_e:
			while(curr != NULL) {
				if(curr->indextype == string_e && !strcmp(curr->name, index.strval))
					return curr;
				curr = curr->next;
			}
			break;
		default:
			// to keep the compiler happy; this will already be caught by previous switch
			break;
	}
	return NULL;
}
ehretval_t array_get(ehvar_t **array, ehretval_t index) {
	ehvar_t *curr;
	ehretval_t ret;

	curr = array_getmember(array, index);
	if(curr == NULL)
		ret.type = null_e;
	else {
		SETRETFROMVAR(curr);
	}
	return ret;
}
int array_count(ehvar_t **array) {
	// count the members of an array
	ehvar_t *curr;
	int i, count = 0;

	for(i = 0; i < VARTABLE_S; i++) {
		curr = array[i];
		while(curr != NULL) {
			count++;
			curr = curr->next;
		}
	}
	return count;
}
/*
 * Command line arguments
 */
void eh_setarg(int argc, char **argv) {
	ehvar_t *argc_v;
	ehvar_t *argv_v;
	int i;
	ehretval_t ret, index;

	// insert argc
	argc_v = Malloc(sizeof(ehvar_t));
	argc_v->value.type = int_e;
	// global scope
	argc_v->scope = 0;
	argc_v->name = "argc";
	// argc - 1, because argv[0] is ehi itself
	argc_v->value.intval = argc - 1;
	insert_variable(argc_v);

	// insert argv
	argv_v = Malloc(sizeof(ehvar_t));
	argv_v->value.type = array_e;
	argv_v->scope = 0;
	argv_v->name = "argv";
	argv_v->value.arrval = Calloc(VARTABLE_S, sizeof(ehvar_t *));

	// all members of argv are strings
	ret.type = string_e;
	index.type = int_e;
	for(i = 1; i < argc; i++) {
		index.intval = i - 1;
		ret.strval = argv[i];
		array_insert_retval(argv_v->value.arrval, index, ret);
	}
	insert_variable(argv_v);
}

/* Hash function */
// This hash function is used because it allows scope to be taken into account for the hash, improving performance for recursive functions. (Otherwise, variables in each function call, with the same names, would get the same hash for every case.)
// The EH interpreter is not dependent on any particular hash implementation

// Bob Jenkins' hash
typedef  unsigned  int  ub4;   /* unsigned 4-byte quantities */
typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.
See http://burtleburtle.net/bob/hash/evahash.html
--------------------------------------------------------------------
*/

unsigned int hash(char *k, int scope)
{
	ub4 initval = (ub4) scope;
	size_t len = strlen(k);
   register ub4 a,b,c;
   register ub4 len2;

   /* Set up the internal state */
   len2 = len;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   c = initval;         /* the previous hash value */

   /*-------------------------------- handle most of the key */
   while (len2 >= 12)
   {
      a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
      b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
      c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
      mix(a,b,c);
      k += 12; len2 -= 12;
   }

   /*--------------------------------- handle the last 11 bytes */
   c += len;
   switch(len2)          /* all the case statements fall through */
   {
   case 11: c+=((ub4)k[10]<<24);
   case 10: c+=((ub4)k[9]<<16);
   case 9 : c+=((ub4)k[8]<<8);
      /* the first byte of c is reserved for the length */
   case 8 : b+=((ub4)k[7]<<24);
   case 7 : b+=((ub4)k[6]<<16);
   case 6 : b+=((ub4)k[5]<<8);
   case 5 : b+=k[4];
   case 4 : a+=((ub4)k[3]<<24);
   case 3 : a+=((ub4)k[2]<<16);
   case 2 : a+=((ub4)k[1]<<8);
   case 1 : a+=k[0];
     /* case 0: nothing left to add */
   }
   mix(a,b,c);
   /*--------------------------------------- report the result */
   return c % VARTABLE_S;
}
