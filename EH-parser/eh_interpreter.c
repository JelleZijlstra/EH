/*
 * eh_interpreter.c
 * Jelle Zijlstra, December 2011
 *
 * Implements a Lex- and Yacc-based interpreter for the EH scripting language.
 */
#include "eh_interpreter.h"

// indicate that we're returning
static bool returning = false;
// number of loops we're currently in
static int inloop = 0;
static int breaking = 0;
static bool continuing = 0;

// current object, gets passed around
static ehcontext_t newcontext = NULL;
int scope = 0;
static void make_arglist(int *argcount, eharg_t **arglist, ehretval_t *node);
static ehretval_t int_arrow_get(ehretval_t operand1, ehretval_t operand2);
static ehretval_t string_arrow_get(ehretval_t operand1, ehretval_t operand2);
static void int_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue);
static void string_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue);

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
		func->f.type = lib_e;
		func->f.ptr = libfuncs[i].code;
		// other fields are irrelevant
		insert_function(func);
	}
	return;
}
void eh_exit(void) {
	return;
}

ehretval_t execute(ehretval_t *node, ehcontext_t context) {
	// variables used
	ehretval_t *node2;
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
		case string_e:
			ret.type = string_e;
			ret.stringval = node->stringval;
			break;
		case int_e:
			ret.type = int_e;
			ret.intval = node->intval;
			break;
		case type_e:
			ret.type = type_e;
			ret.typeval = node->typeval;
			break;
		case bool_e:
			ret.type = bool_e;
			ret.boolval = node->boolval;
			break;
		case accessor_e:
			ret.type = accessor_e;
			ret.accessorval = node->accessorval;
			break;
		case magicvar_e:
			ret.type = magicvar_e;
			ret.magicvarval = node->magicvarval;
			break;
		case null_e:
			break;
		case op_e:
			//printf("Executing opcode: %d\n", node->opval->op);
			switch(node->opval->op) {
			/*
			 * Unary operators
			 */
				case T_ECHO:
					ret = execute(node->opval->paras[0], context);
					switch(ret.type) {
						case string_e:
							printf("%s\n", ret.stringval);
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
							eh_error_type("echo operator", ret.type, enotice_e);
							break;
					}
					break;
				case '@': // type casting
					operand1 = execute(node->opval->paras[0], context);
					operand2 = execute(node->opval->paras[1], context);
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
							eh_error_type("typecast", operand1.typeval, eerror_e);
							break;
					}
					break;
				case T_COUNT:
					operand1 = execute(node->opval->paras[0], context);
					ret.type = int_e;
					switch(operand1.type) {
						case int_e:
							ret.intval = sizeof(int) * 8;
							break;
						case string_e:
							ret.intval = strlen(operand1.stringval);
							break;
						case array_e:
							ret.intval = array_count(operand1.arrayval);
							break;
						case null_e:
							ret.intval = 0;
							break;
						case bool_e:
							ret.intval = 0;
							break;
						default:
							eh_error_type("count operator", operand1.type, eerror_e);
							break;
					}
					break;
				case '~': // bitwise negation
					operand1 = eh_xtoi(execute(node->opval->paras[0], context));
					if(operand1.type != int_e)
						eh_error_type("bitwise negation", operand1.type, eerror_e);
					else {
						ret.type = int_e;
						ret.intval = ~operand1.intval;
					}
					break;
				case T_NEGATIVE: // sign change
					operand1 = eh_xtoi(execute(node->opval->paras[0], context));
					if(operand1.type != int_e)
						eh_error_type("negation", operand1.type, eerror_e);
					else {
						ret.type = int_e;
						ret.intval = -operand1.intval;
					}
					break;
				case '!': // Boolean not
					ret = eh_xtobool(execute(node->opval->paras[0], context));
					ret.boolval = !ret.boolval;
					break;
				case T_GLOBAL: // global variable declaration
					name = node->opval->paras[0]->stringval;
					var = get_variable(name, 0);
					if(var == NULL) {
						eh_error_unknown("global variable", name, enotice_e);
						break;
					}
					member = Malloc(sizeof(ehvar_t));
					member->name = name;
					member->scope = scope;
					member->value.type = reference_e;
					member->value.referenceval = &var->value;
					insert_variable(member);
					break;
			/*
			 * Control flow
			 */
				case T_IF:
					if(eh_xtobool(execute(node->opval->paras[0], context)).boolval) {
						ret = execute(node->opval->paras[1], context);
						if(returning)
							return ret;
					}
					else if(node->opval->nparas == 3)
						ret = execute(node->opval->paras[2], context);
					break;
				case T_WHILE:
					inloop++;
					breaking = 0;
					while(eh_xtobool(execute(node->opval->paras[0], context)).boolval) {
						ret = execute(node->opval->paras[1], context);
						LOOPCHECKS;
					}
					inloop--;
					break;
				case T_FOR:
					inloop++;
					breaking = 0;
					// get the count
					operand1 = eh_xtoi(execute(node->opval->paras[0], context));
					if(operand1.type != int_e) {
						eh_error_type("count", operand1.type, eerror_e);
						break;
					}
					count = operand1.intval;
					if(node->opval->nparas == 2) {
						// "for 5; do stuff; endfor" construct
						for(i = 0; i < count; i++) {
							ret = execute(node->opval->paras[1], context);
							LOOPCHECKS;
						}
					}
					else {
						// "for 5 count i; do stuff; endfor" construct
						name = node->opval->paras[1]->stringval;
						var = get_variable(name, scope);
						// variable is not yet set, so set it
						if(var == NULL) {
							var = Malloc(sizeof(ehvar_t));
							var->name = node->opval->paras[1]->stringval;
							var->scope = scope;
							insert_variable(var);
						}
						// count variable always gets to be an int
						var->value.type = int_e;
						for(var->value.intval = 0; var->value.intval < count; var->value.intval++) {
							ret = execute(node->opval->paras[2], context);
							LOOPCHECKS;
						}
					}
					inloop--;
					break;
			/*
			 * Miscellaneous
			 */
				case T_SEPARATOR:
					ret = execute(node->opval->paras[0], context);
					if(returning || breaking || continuing)
						return ret;
					ret = execute(node->opval->paras[1], context);
					break;
				case T_EXPRESSION: // wrapper for special case
					ret = execute(node->opval->paras[0], context);
					break;
				case T_CALL: // call: execute argument and discard it
					execute(node->opval->paras[0], context);
					break;
				case T_RET: // return from a function or the program
					returning = true;
					ret = execute(node->opval->paras[0], context);
					break;
				case T_BREAK: // break out of a loop
					if(node->opval->nparas == 0)
						operand1 = (ehretval_t){int_e, {1}};
					else
						operand1 = eh_xtoi(execute(node->opval->paras[0], context));
					if(operand1.type != int_e)
						break;
					// break as many levels as specified by the argument
					if(operand1.intval > inloop) {
						eh_error_looplevels("Cannot break", operand1.intval);
						break;
					}
					breaking = operand1.intval;
					break;
				case T_CONTINUE: // continue in a loop
					if(node->opval->nparas == 0)
						operand1 = (ehretval_t){int_e, {1}};
					else
						operand1 = eh_xtoi(execute(node->opval->paras[0], context));
					if(operand1.type != int_e)
						break;
					// break as many levels as specified by the argument
					if(operand1.intval > inloop) {
						eh_error_looplevels("Cannot continue", operand1.intval);
						break;
					}
					continuing = operand1.intval;
					break;
			/*
			 * Object access
			 */
				case ':': // function call
					operand1 = execute(node->opval->paras[0], context);
					// operand1 will be either a string (indicating a normal function call) or a func_e (indicating a method or closure call)
					if(operand1.type == string_e) {
						name = operand1.stringval;
						func = get_function(name);
						//printf("Calling function %s at scope %d\n", node->opval->paras[0]->id.name, scope);
						if(func == NULL) {
							eh_error_unknown("function", name, efatal_e);
							break;
						}
						ret = call_function(&func->f, node->opval->paras[1], context, context);
					}
					else if(operand1.type == func_e) {
						ret = call_function(operand1.funcval, node->opval->paras[1], context, newcontext);
					}
					break;
				case T_ACCESSOR: // array access, and similar stuff for other types
					operand1 = execute(node->opval->paras[0], context);
					if(node->opval->paras[1]->accessorval == dot_e) {
						// object access
						if(operand1.type != object_e) {
							eh_error_type("object access", operand1.type, eerror_e);
							break;
						}
						name = execute(node->opval->paras[0], context).stringval;
						ret = class_get(operand1.objectval, name, context);
						if(ret.type == null_e) {
							eh_error_unknown("object member", name, eerror_e);
							break;
						}
						newcontext = operand1.objectval;
					} else if(node->opval->paras[1]->accessorval == arrow_e) {
						// "array" access
						operand2 = execute(node->opval->paras[2], context);
						switch(operand1.type) {
							case int_e:
								ret = int_arrow_get(operand1, operand2);
								break;
							case string_e:
								ret = string_arrow_get(operand1, operand2);
								break;
							case array_e:
								// array access to an array works as expected.
								ret = array_get(operand1.arrayval, operand2);
								break;
							default:
								eh_error_type("array access", operand1.type, eerror_e);
								break;
						}
					}
					else
						eh_error("Unsupported accessor", efatal_e);
					break;
				case T_NEW: // object declaration
					name = execute(node->opval->paras[0], context).stringval;
					class = get_class(name);
					if(class == NULL) {
						eh_error_unknown("class", name, efatal_e);
						break;
					}
					ret.type = object_e;
					ret.objectval = Malloc(sizeof(ehobj_t));
					ret.objectval->class = name;
					ret.objectval->members = Calloc(VARTABLE_S, sizeof(ehclassmember_t *));
					ehclassmember_t *newmember;
					for(i = 0; i < VARTABLE_S; i++) {
						classmember = class->members[i];
						while(classmember != NULL) {
							newmember = Malloc(sizeof(ehclassmember_t));
							// copy the whole thing over
							*newmember = *classmember;
							newmember->next = ret.objectval->members[i];
							ret.objectval->members[i] = newmember;
							classmember = classmember->next;
						}
					}
					break;
			/*
			 * Object definitions
			 */
				case T_FUNC: // function definition
					name = node->opval->paras[0]->stringval;
					//printf("Defining function %s with %d paras\n", node->opval->paras[0]->id.name, node->opval->nparas);
					func = get_function(name);
					// function definition
					if(func != NULL) {
						eh_error_redefine("function", name, efatal_e);
						break;
					}
					func = Malloc(sizeof(ehfunc_t));
					func->name = name;
					// determine argcount
					make_arglist(&func->f.argcount, &func->f.args, node->opval->paras[1]);
					func->f.code = node->opval->paras[2];
					func->f.type = user_e;
					insert_function(func);
					break;
				case T_CLASS: // class declaration
					operand1 = execute(node->opval->paras[0], context);
					class = get_class(operand1.stringval);
					if(class != NULL) {
						eh_error_redefine("class", operand1.stringval, efatal_e);
						break;
					}
					class = Malloc(sizeof(ehclass_t));
					class->name = operand1.stringval;
					class->members = Calloc(VARTABLE_S, sizeof(ehclassmember_t *));
					// insert class members
					node = node->opval->paras[1];
					while(node != NULL) {
						if(node->type == op_e && node->opval->op == ',') {
							class_insert(class->members, node->opval->paras[0], context);						
							node = node->opval->paras[1];
						}
						else {
							class_insert(class->members, node, context);
							break;
						}
					}
					insert_class(class);
					break;
				case '[': // array declaration
					ret.type = array_e;
					ret.arrayval = Calloc(VARTABLE_S, sizeof(ehvar_t *));
					// need to count array members first, because they are reversed in our node.
					// That's not necessary with functions (where the situation is analogous), because the reversals that happen when parsing the prototype argument list and parsing the argument list in a call cancel each other out.
					node2 = node->opval->paras[0];
					count = 0;
					while(node2->opval->nparas != 0) {
						count++;
						node2 = node2->opval->paras[0];
					}
					node2 = node->opval->paras[0];
					while(node2->opval->nparas != 0) {
						array_insert(ret.arrayval, node2->opval->paras[1], --count, context);
						node2 = node2->opval->paras[0];
					}
					break;
			/*
			 * Binary operators
			 */
				case '=': // equality
					operand1 = execute(node->opval->paras[0], context);
					operand2 = execute(node->opval->paras[1], context);
					ret = eh_looseequals(operand1, operand2);
					break;
				case T_SE: // strict equality
					operand1 = execute(node->opval->paras[0], context);
					operand2 = execute(node->opval->paras[1], context);
					ret = eh_strictequals(operand1, operand2);
					break;
				EH_INTBOOL_CASE('>', >) // greater-than
				EH_INTBOOL_CASE('<', <) // lesser-than
				EH_INTBOOL_CASE(T_GE, >=) // greater-than or equal
				EH_INTBOOL_CASE(T_LE, <=) // lesser-than or equal
				EH_INTBOOL_CASE(T_NE, !=) // not equal
				// doing addition on two strings performs concatenation
				case '+':
					operand1 = execute(node->opval->paras[0], context);
					operand2 = execute(node->opval->paras[1], context);
					if(IS_STRING(operand1) && IS_STRING(operand2)) {
						// concatenate them
						ret.type = string_e;
						size_t len1, len2;
						len1 = strlen(operand1.stringval);
						len2 = strlen(operand2.stringval);
						ret.stringval = Malloc(len1 + len2 + 1);
						strcpy(ret.stringval, operand1.stringval);
						strcpy(ret.stringval + len1, operand2.stringval);
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
				EH_INT_CASE('-', -) // subtraction
				EH_INT_CASE('*', *) // multiplication
				EH_INT_CASE('/', /) // division
				EH_INT_CASE('%', %) // modulo
				EH_INT_CASE('&', &) // bitwise AND
				EH_INT_CASE('^', ^) // bitwise XOR
				EH_INT_CASE('|', |) // bitwise OR
				case T_AND: // AND; use short-circuit operation
					operand1 = eh_xtobool(execute(node->opval->paras[0], context));
					if(!operand1.boolval)
						ret = operand1;
					else
						ret = eh_xtobool(execute(node->opval->paras[1], context));
					break;
				case T_OR: // OR; use short-circuit operation
					operand1 = eh_xtobool(execute(node->opval->paras[0], context));
					if(operand1.boolval)
						ret = operand1;
					else
						ret = eh_xtobool(execute(node->opval->paras[1], context));
					break;
				case T_XOR:
					operand1 = eh_xtobool(execute(node->opval->paras[0], context));
					operand2 = eh_xtobool(execute(node->opval->paras[1], context));
					ret.type = bool_e;
					if((operand1.boolval && operand2.boolval) || (!operand1.boolval && !operand2.boolval))
						ret.boolval = false;
					else
						ret.boolval = true;
					break;
			/*
			 * Variable manipulation
			 */
				case T_LVALUE:
					/*
					 * Get an lvalue. This case normally returns an
					 * ehretval_t of type reference_e: a pointer to an
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
					operand1 = execute(node->opval->paras[0], context);
					ret.referenceval = NULL;
					switch(node->opval->nparas) {
						case 1:
							if(operand1.type == magicvar_e) {
								eh_error("Cannot use magic variable in scalar context", eerror_e);
								break;
							}
							var = get_variable(operand1.stringval, scope);
							// dereference variable
							if(var != NULL) {
								ret.type = reference_e;
								ret.referenceval = &var->value;
							}
							/*
							 * If there is no variable of this name, and it is a 
							 * simple access, we use NULL as the referenceval.
							 */
							break;
						case 3:
							if(node->opval->paras[1]->accessorval == arrow_e) {										
								if(operand1.type == magicvar_e) {
									eh_error("Cannot use magic variable in array context", eerror_e);
									break;
								}
								var = get_variable(operand1.stringval, scope);
								if(var == NULL) {
									eh_error("Cannot access member of non-existing variable", eerror_e);
									ret.referenceval = (ehretval_t *) 0x1;
								}
								if(var->value.type == array_e) {
									operand1 = execute(node->opval->paras[2], context);							
									member = array_getmember(var->value.arrayval, operand1);
									// if there is no member yet, insert it with a null value
									if(member == NULL) {
										member = array_insert_retval(var->value.arrayval, operand1, ret);
									}
									ret.type = reference_e;
									ret.referenceval = &member->value;
								}
								else
									ret.referenceval = &var->value;
							} 
							else if(node->opval->paras[1]->accessorval == dot_e)
								ret = object_access(operand1, node->opval->paras[2], context);
							else
								eh_error("Unsupported accessor", efatal_e);
							break;
					}
					break;
				case T_SET:
					operand1 = execute(node->opval->paras[0], context);
					operand2 = execute(node->opval->paras[1], context);
					if(operand1.type == null_e) {
						if(operand1.referenceval == NULL || operand1.referenceval == (ehretval_t *) 0x1) {
							// set new variable
							var = Malloc(sizeof(ehvar_t));
							var->name = node->opval->paras[0]->opval->paras[0]->stringval;
							var->scope = scope;
							var->value = operand2;
							insert_variable(var);
						}
						else if(operand1.referenceval == (ehretval_t *) 0x1) {
							// do nothing; T_LVALUE will already have complained
						}
						else {
							// operand 1 is a pointer to the variable modified, operand 2 is the value set to, operand 3 is the index
							operand3 = execute(node->opval->paras[0]->opval->paras[2], context);
							switch(operand1.referenceval->type) {
								case int_e:
									int_arrow_set(operand1, operand3, operand2);
									break;
								case string_e:
									string_arrow_set(operand1, operand3, operand2);
									break;
								default:
									eh_error_type("array access", operand1.referenceval->type, eerror_e);
									break;
							}
						}
					}
					else {
						while(operand1.type == reference_e && operand1.referenceval->type == reference_e)
							operand1 = *(operand1.referenceval);
						// set variable
						*operand1.referenceval = operand2;
					}
					break;
				case T_MINMIN:
					operand1 = execute(node->opval->paras[0], context);
					if(operand1.type == null_e)
						eh_error("Cannot set with -- operator", eerror_e);
					else switch(operand1.referenceval->type) {
						case int_e:
							operand1.referenceval->intval--;
							break;
						default:
							eh_error_type("-- operator", operand1.referenceval->type, eerror_e);
							break;
					}
					break;
				case T_PLUSPLUS:
					operand1 = execute(node->opval->paras[0], context);
					if(operand1.type == null_e)
						eh_error("Cannot set with ++ operator", eerror_e);
					else switch(operand1.referenceval->type) {
						case int_e:
							operand1.referenceval->intval++;
							break;
						default:
							eh_error_type("++ operator", operand1.referenceval->type, eerror_e);
							break;
					}
					break;
				case T_REFERENCE: // reference declaration
					ret = execute(node->opval->paras[0], context);
					if(ret.type != reference_e)
						eh_error("Unable to create reference", eerror_e);
					break;
				case '$': // variable dereference
					ret = execute(node->opval->paras[0], context);
					if(ret.type == null_e) {
						if(ret.referenceval == NULL || ret.referenceval == (ehretval_t *) 0x1)
							break;
						// get operands
						operand2 = execute(node->opval->paras[0]->opval->paras[2], context);
						if(operand2.type == reference_e)
							ret = *ret.referenceval;
						switch(ret.referenceval->type) {
							case int_e:
								ret = int_arrow_get(*ret.referenceval, operand2);
								break;
							case string_e:
								ret = string_arrow_get(*ret.referenceval, operand2);
								break;
							default:
								eh_error_type("array-type dereference", ret.referenceval->type, eerror_e);
								break;
						}
					}
					else while(ret.type == reference_e)
						ret = *ret.referenceval;
					break;
				default:
					eh_error_int("Unexpected opcode", node->opval->op, efatal_e);
					break;
			}
			break;
		default:
			eh_error_int("Unsupported node type", node->type, efatal_e);
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
static void make_arglist(int *argcount, eharg_t **arglist, ehretval_t *node) {
	*argcount = 0;
	// traverse linked list to determine argument count
	ehretval_t *tmp;
	int currarg = 0;

	tmp = node;
	while(tmp->opval->nparas != 0) {
		currarg++;
		tmp = tmp->opval->paras[0];
	}
	*argcount = currarg;
	// if there are no arguments, the arglist can be NULL
	if(currarg)
		*arglist = Malloc(currarg * sizeof(eharg_t));
	else
		*arglist = NULL;
	// add arguments to arglist
	tmp = node;
	currarg = 0;
	while(tmp->opval->nparas != 0) {
		(*arglist)[currarg].name = tmp->opval->paras[1]->stringval;
		currarg++;
		tmp = tmp->opval->paras[0];
	}
}
ehretval_t call_function(ehfm_t *f, ehretval_t *args, ehcontext_t context, ehcontext_t newcontext) {
	ehretval_t ret;
	ehvar_t *var;

	ret.type = null_e;
	if(f->type == lib_e) {
		// library function
		f->ptr(args, &ret, context);
		return ret;
	}
	int i = 0;
	// set parameters as necessary
	if(f->args == NULL) {
		if(args->opval->nparas != 0) {
			eh_error_argcount(f->argcount, 1);
			return ret;
		}
	}
	else while(args->opval->nparas != 0) {
		var = Malloc(sizeof(ehvar_t));
		var->name = f->args[i].name;
		var->scope = scope + 1;
		insert_variable(var);
		i++;
		if(i > f->argcount) {
			eh_error_argcount(f->argcount, i);
			return ret;
		}
		var->value = execute(args->opval->paras[1], context);;
		args = args->opval->paras[0];
	}
	// functions get their own scope (not incremented before because execution of arguments needs parent scope)
	scope++;
	if(f->argcount != i) {
		eh_error_argcount(f->argcount, i);
		return ret;
	}
	// set new context (only useful for methods)
	ret = execute(f->code, newcontext);
	returning = false;
	for(i = 0; i < f->argcount; i++) {
		remove_variable(f->args[i].name, scope);
	}
	scope--;
	return ret;
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
void class_insert(ehclassmember_t **class, ehretval_t *in, ehcontext_t context) {
	// insert a member into a class
	unsigned int vhash;
	ehclassmember_t *member;
	
	member = Malloc(sizeof(ehclassmember_t));
	// rely on standard layout of the input ehretval_t
	member->visibility = in->opval->paras[0]->visibilityval;
	member->name = in->opval->paras[1]->stringval;

	// decide what we got
	switch(in->opval->nparas) {
		case 2: // non-set property: null
			member->value.type = null_e;
			break;
		case 3: // set property
			member->value = execute(in->opval->paras[2], context);
			break;
		case 4: // method
			member->value.type = func_e;
			member->value.funcval = Malloc(sizeof(ehfm_t));
			member->value.funcval->code = in->opval->paras[3];
			make_arglist(&member->value.funcval->argcount, &member->value.funcval->args, in->opval->paras[2]);
			break;
	}

	// insert into hash table
	vhash = hash(member->name, 0);	
	member->next = class[vhash];
	class[vhash] = member;
}
ehclassmember_t *class_getmember(ehobj_t *class, char *name, ehcontext_t context) {
	ehclassmember_t *curr;
	unsigned int vhash;
	
	vhash = hash(name, 0);
	curr = class->members[vhash];
	while(curr != NULL) {
		if(!strcmp(curr->name, name)) {
			// we found it; now check visibility
			switch(curr->visibility) {
				case public_e:
					return curr;
				case private_e:
					// if context is NULL, we're never going to get private stuff
					if(context == NULL)
						return NULL;
					// check context
					if(ehcontext_compare(class, context))
						return curr;
					else
						return NULL;
			}
		}
		curr = curr->next;
	}
	return curr;
}
ehretval_t class_get(ehobj_t *class, char *name, ehcontext_t context) {
	ehclassmember_t *curr;
	ehretval_t ret;
	
	curr = class_getmember(class, name, context);
	if(curr == NULL)
		ret.type = null_e;
	else
		ret = curr->value;
	return ret;
}
ehretval_t object_access(ehretval_t operand1, ehretval_t *index, ehcontext_t context) {
	ehretval_t label, ret;
	ehvar_t *var;
	ehclassmember_t *classmember;
	ehobj_t *object;

	// default value. Set the referenceval explicitly because of T_LVALUE special conventions
	ret.type = null_e;
	ret.referenceval = NULL;
	
	label = execute(index, context);
	if(label.type != string_e) {
		eh_error_type("object member label", label.type, eerror_e);
		return ret;
	}

	// this dereference
	if(operand1.type == magicvar_e) {
		if(var->value.magicvarval == this_e) {
			if(context == NULL) {
				eh_error("Use of $this outside an object", eerror_e);
				return ret;
			}
			object = context;
		}
		else {
			eh_error_int("Unsupported magicvar", var->value.magicvarval, efatal_e);
			return ret;
		}
	}
	else {
		var = get_variable(operand1.stringval, scope);
		if(var->value.type != object_e) {
			eh_error_type("object access", var->value.type, eerror_e);
			return ret;
		}
		object = var->value.objectval;
	}
	classmember = class_getmember(object, label.stringval, context);
	if(classmember == NULL) {
		eh_error_unknown("object member", label.stringval, eerror_e);
		return ret;
	}
	ret.type = reference_e;
	ret.referenceval = &classmember->value;
	newcontext = object;
	return ret;
}
bool ehcontext_compare(ehcontext_t lock, ehcontext_t key) {
	return !strcmp(lock->class, key->class);
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
		eh_error("Unable to perform type juggling to int", enotice_e);
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
			ret = eh_strtoi(in.stringval);
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
			eh_error_type("typecast to integer", in.type, enotice_e);
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
			ret.stringval = in.stringval;
			break;
		case int_e:
			ret.stringval = eh_itostr(in.intval);
			break;
		case null_e:
			// empty string
			ret.stringval = Malloc(1);
			ret.stringval[0] = '\0';
			break;
		case bool_e:
			if(in.boolval) {
				ret.stringval = Malloc(5);
				strcpy(ret.stringval, "true");
			}
			else {
				ret.stringval = Malloc(6);
				strcpy(ret.stringval, "false");
			}
			break;
		default:
			eh_error_type("typecast to string", in.type, enotice_e);
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
			if(strlen(in.stringval) == 0)
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
ehretval_t eh_looseequals(ehretval_t operand1, ehretval_t operand2) {
	ehretval_t ret;
	ret.type = bool_e;

	if(IS_INT(operand1) && IS_INT(operand2)) {
		ret.boolval = (operand1.intval == operand2.intval);
	}
	else if(IS_STRING(operand1) && IS_STRING(operand2)) {
		ret.boolval = !strcmp(operand1.stringval, operand2.stringval);
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
	return ret;
}
ehretval_t eh_strictequals(ehretval_t operand1, ehretval_t operand2) {
	ehretval_t ret;
	ret.type = bool_e;

	if(IS_INT(operand1) && IS_INT(operand2)) {
		ret.boolval = (operand1.intval == operand2.intval);
	}
	else if(IS_STRING(operand1) && IS_STRING(operand2)) {
		ret.boolval = !strcmp(operand1.stringval, operand2.stringval);
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
	return ret;
}
/*
 * Arrays
 */
void array_insert(ehvar_t **array, ehretval_t *in, int place, ehcontext_t context) {
	unsigned int vhash;
	ehretval_t var;
	ehretval_t label;

	// new array member
	ehvar_t *member = Malloc(sizeof(ehvar_t));

	/*
	 * We'll assume we're always getting a correct ehretval_t *, referring to a
	 * T_ARRAYMEMBER token. If there is 1 parameter, that means it's a
	 * non-labeled array member, which we'll give an integer array index; if
	 * there are 2, we'll either use the integer array index or a hash of the
	 * string index.
	 */
	if(in->opval->nparas == 1) {
		// if there is no explicit key, simply use the place argument
		vhash = place % VARTABLE_S;
		var = execute(in->opval->paras[0], context);
		member->indextype = int_e;
		member->index = place;
	}
	else {
		label = execute(in->opval->paras[0], context);
		switch(label.type) {
			case int_e:
				vhash = label.intval % VARTABLE_S;
				member->indextype = int_e;
				member->index = label.intval;
				break;
			case string_e:
				vhash = hash(label.stringval, 0);
				member->indextype = string_e;
				member->name = label.stringval;
				break;
			default:
				eh_error_type("array member label", label.type, enotice_e);
				free(member);
				return;
		}
		var = execute(in->opval->paras[1], context);
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
		default: // to keep the compiler happy
			break;
	}
	*currptr = member;
	return;
}
ehvar_t *array_insert_retval(ehvar_t **array, ehretval_t index, ehretval_t ret) {
	// Inserts a member into an array. Assumes that the member is not yet present in the array.
	ehvar_t *new;
	unsigned int vhash = 0;

	new = Malloc(sizeof(ehvar_t));
	new->indextype = index.type;
	switch(index.type) {
		case int_e:
			vhash = index.intval % VARTABLE_S;
			new->index = index.intval;
			break;
		case string_e:
			vhash = hash(index.stringval, 0);
			new->name = index.stringval;
			break;
		default:
			eh_error_type("array index", index.type, enotice_e);
			free(new);
			return NULL;
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
			vhash = hash(index.stringval, 0);
			break;
		default:
			eh_error_type("array index", index.type, enotice_e);
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
				if(curr->indextype == string_e && !strcmp(curr->name, index.stringval))
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
			if(curr->value.type != null_e)
				count++;
			curr = curr->next;
		}
	}
	return count;
}
/*
 * Variants of array access
 */
static ehretval_t int_arrow_get(ehretval_t operand1, ehretval_t operand2) {
	ehretval_t ret;
	int mask;

	ret.type = null_e;
	// "array" access to integer returns the nth bit of the integer; for example (assuming sizeof(int) == 32), (2 -> 30) == 1, (2 -> 31) == 0
	if(operand2.type != int_e) {
		eh_error_type("bitwise access to integer", operand2.type, enotice_e);
		return ret;
	}
	if(operand2.intval >= sizeof(int) * 8) {
		eh_error_int("Identifier too large for bitwise access to integer", operand2.intval, enotice_e);
		return ret;
	}
	// get mask
	mask = 1 << (sizeof(int) * 8 - 1);
	mask >>= operand2.intval;
	// apply mask
	ret.intval = (operand1.intval & mask) >> (sizeof(int) * 8 - 1 - mask);
	ret.type = int_e;
	return ret;
}
static ehretval_t string_arrow_get(ehretval_t operand1, ehretval_t operand2) {
	ehretval_t ret;
	int count;

	ret.type = null_e;

	// "array" access to a string returns an integer representing the nth character.
	// In the future, perhaps replace this with a char datatype or with a "shortstring" datatype representing strings up to 3 or even 4 characters long
	if(operand2.type != int_e) {
		eh_error_type("character access to string", operand2.type, enotice_e);
		return ret;
	}
	count = strlen(operand1.stringval);
	if(operand2.intval >= count) {
		eh_error_int("Identifier too large for character access to string", operand2.intval, enotice_e);
		return ret;
	}
	// get the nth character
	ret.intval = operand1.stringval[operand2.intval];
	ret.type = int_e;
	return ret;
}
static void int_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue) {
	int mask;

	if(index.type != int_e) {
		eh_error_type("bitwise access to integer", index.type, enotice_e);
		return;
	}
	if(index.intval >= sizeof(int) * 8) {
		eh_error_int("Identifier too large for bitwise access to integer", index.intval, enotice_e);
		return;
	}
	// get mask
	mask = (1 << (sizeof(int) * 8 - 1)) >> index.intval;
	if(eh_xtobool(rvalue).boolval)
		input.referenceval->intval |= mask;
	else {
		mask = ~mask;
		input.referenceval->intval &= mask;
	}
	return;
}
static void string_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue) {
	int count;

	if(rvalue.type != int_e) {
		eh_error_type("character access to string", rvalue.type, enotice_e);
		return;
	}
	if(index.type != int_e) {
		eh_error_type("setting a character in a string", index.type, enotice_e);
		return;
	}
	count = strlen(input.referenceval->stringval);
	if(index.intval >= count) {
		eh_error_int("Identifier too large for character access to string", index.intval, enotice_e);
		return;
	}
	// get the nth character
	input.referenceval->stringval[index.intval] = rvalue.intval;
	return;
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
	argv_v->value.arrayval = Calloc(VARTABLE_S, sizeof(ehvar_t *));

	// all members of argv are strings
	ret.type = string_e;
	index.type = int_e;
	for(i = 1; i < argc; i++) {
		index.intval = i - 1;
		ret.stringval = argv[i];
		array_insert_retval(argv_v->value.arrayval, index, ret);
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
