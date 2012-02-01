/*
 * eh_interpreter.c
 * Jelle Zijlstra, December 2011
 *
 * Implements a Lex- and Yacc-based interpreter for the EH scripting language.
 */
#include "eh.h"
#include "eh_libfuncs.h"
#include <cctype>

// number of loops we're currently in
bool returning = false;
static int inloop = 0;
static int breaking = 0;
static int continuing = 0;
ehvar_t *vartable[VARTABLE_S];
ehfunc_t *functable[VARTABLE_S];
ehclass_t *classtable[VARTABLE_S];

// current object, gets passed around
static ehcontext_t newcontext = NULL;
int scope = 0;
static void make_arglist(int *argcount, eharg_t **arglist, ehretval_t *node);
static ehretval_t int_arrow_get(ehretval_t operand1, ehretval_t operand2);
static ehretval_t string_arrow_get(ehretval_t operand1, ehretval_t operand2);
static ehretval_t range_arrow_get(ehretval_t operand1, ehretval_t operand2);
static void int_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue);
static void string_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue);
static void range_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue);
// helper functions
void print_retval(const ehretval_t in);
ehretval_t eh_count(const ehretval_t in);
ehretval_t eh_op_tilde(ehretval_t in);
ehretval_t eh_op_uminus(ehretval_t in);
ehretval_t eh_op_plus(ehretval_t operand1, ehretval_t operand2);
void eh_op_global(const char *name, ehcontext_t context);
ehretval_t eh_op_command(const char *name, ehretval_t *node, ehcontext_t context);
ehretval_t eh_op_for(opnode_t *op, ehcontext_t context);
ehretval_t eh_op_while(ehretval_t **paras, ehcontext_t context);
ehretval_t eh_op_as(opnode_t *op, ehcontext_t context);
ehretval_t eh_op_new(const char *name);
void eh_op_continue(opnode_t *op, ehcontext_t context);
void eh_op_break(opnode_t *op, ehcontext_t context);
ehretval_t eh_op_array(ehretval_t *node, ehcontext_t context);
void eh_op_declarefunc(ehretval_t **paras);
ehretval_t eh_op_declareclosure(ehretval_t **paras);
void eh_op_declareclass(ehretval_t **paras, ehcontext_t context);
ehretval_t eh_op_switch(ehretval_t **paras, ehcontext_t context);
ehretval_t eh_op_given(ehretval_t **paras, ehcontext_t context);
ehretval_t eh_op_colon(ehretval_t **paras, ehcontext_t context);
ehretval_t eh_op_lvalue(opnode_t *op, ehcontext_t context);
ehretval_t eh_op_dollar(ehretval_t *node, ehcontext_t context);
void eh_op_set(ehretval_t **paras, ehcontext_t context);
ehretval_t eh_op_accessor(ehretval_t **paras, ehcontext_t context);

ehretval_t eh_make_range(const int min, const int max);

#define LIBFUNCENTRY(f) {ehlf_ ## f, #f},
// library functions supported by ehi
ehlibfunc_t libfuncs[] = {
	LIBFUNCENTRY(getinput)
	LIBFUNCENTRY(printvar)
	LIBFUNCENTRY(is_null)
	LIBFUNCENTRY(is_string)
	LIBFUNCENTRY(is_int)
	LIBFUNCENTRY(is_bool)
	LIBFUNCENTRY(is_array)
	LIBFUNCENTRY(is_object)
	LIBFUNCENTRY(is_range)
	LIBFUNCENTRY(is_float)
	LIBFUNCENTRY(class_is)
	LIBFUNCENTRY(get_type)
	LIBFUNCENTRY(include)
	LIBFUNCENTRY(pow)
	{NULL, NULL}
};

/*
 * macros for interpreter behavior
 */
// take ints, return an int
#define EH_INT_CASE(token, operator) case token: \
	operand1 = eh_xtoint(eh_execute(node->opval->paras[0], context)); \
	operand2 = eh_xtoint(eh_execute(node->opval->paras[1], context)); \
	if(operand1.type == int_e && operand2.type == int_e) { \
		ret.type = int_e; \
		ret.intval = (operand1.intval operator operand2.intval); \
	} \
	else \
		eh_error_types(#operator, operand1.type, operand2.type, eerror_e); \
	break;
// take ints or floats, return an int or float
#define EH_FLOATINT_CASE(token, operator) case token: \
	operand1 = eh_execute(node->opval->paras[0], context); \
	operand2 = eh_execute(node->opval->paras[1], context); \
	if(operand1.type == float_e && operand2.type == float_e) { \
		ret.type = float_e; \
		ret.floatval = (operand1.floatval operator operand2.floatval); \
	} \
	else { \
		operand1 = eh_xtoint(operand1); \
		operand2 = eh_xtoint(operand2); \
		if(operand1.type == int_e && operand2.type == int_e) { \
			ret.type = int_e; \
			ret.intval = (operand1.intval operator operand2.intval); \
		} \
		else \
			eh_error_types(#operator, operand1.type, operand2.type, eerror_e); \
	} \
	break;
// take ints or floats, return a bool
#define EH_INTBOOL_CASE(token, operator) case token: \
	operand1 = eh_execute(node->opval->paras[0], context); \
	operand2 = eh_execute(node->opval->paras[1], context); \
	if(operand1.type == float_e && operand2.type == float_e) { \
		ret.type = bool_e; \
		ret.boolval = (operand1.floatval operator operand2.floatval); \
	} \
	else { \
		operand1 = eh_xtoint(operand1); \
		operand2 = eh_xtoint(operand2); \
		if(operand1.type == int_e && operand2.type == int_e) { \
			ret.type = bool_e; \
			ret.boolval = (operand1.intval operator operand2.intval); \
		} \
		else \
			eh_error_types(#operator, operand1.type, operand2.type, eerror_e); \
	} \
	break;
// take bools, return a bool
#define EH_BOOL_CASE(token, operator) case token: \
	operand1 = eh_xtobool(eh_execute(node->opval->paras[0], context)); \
	operand2 = eh_xtobool(eh_execute(node->opval->paras[1], context)); \
	ret.type = bool_e; \
	ret.boolval = (operand1.boolval operator operand2.boolval); \
	break;

/*
 * Stuff to be done in a loop
 */
#define LOOPCHECKS { \
	if(returning) { \
		inloop--; \
		return ret; \
	} \
	if(breaking) { \
		breaking--; \
		inloop--; \
		return ret; \
	} \
	if(continuing > 1) { \
		continuing--; \
		inloop--; \
		return ret; \
	} \
	else if(continuing) { \
		continuing = 0; \
		continue; \
	} \
	}

/*
 * Functions executed before and after the program itself is executed.
 */
void eh_init(void) {
	int i;
	ehfunc_t *func;

	for(i = 0; libfuncs[i].code != NULL; i++) {
		func = (ehfunc_t *) Malloc(sizeof(ehfunc_t));
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

/*
 * Main execution function
 */
ehretval_t eh_execute(const ehretval_t *node, const ehcontext_t context) {
	// variables used
	ehretval_t ret, operand1, operand2;
	// default
	ret.type = null_e;

	// empty statements produce a null node
	if(node == NULL)
		return ret;
	if(node->type == op_e)
		switch(node->opval->op) {
		/*
		 * Unary operators
		 */
			case T_ECHO:
				print_retval(eh_execute(node->opval->paras[0], context));
				printf("\n");
				break;
			case T_PUT:
				print_retval(eh_execute(node->opval->paras[0], context));
				break;
			case '@': // type casting
				operand2 = eh_execute(node->opval->paras[1], context);
				ret = eh_cast(node->opval->paras[0]->typeval, operand2);
				break;
			case T_COUNT:
				ret = eh_count(eh_execute(node->opval->paras[0], context));
				break;
			case '~': // bitwise negation
				ret = eh_op_tilde(eh_execute(node->opval->paras[0], context));
				break;
			case T_NEGATIVE: // sign change
				ret = eh_op_uminus(eh_execute(node->opval->paras[0], context));
				break;
			case '!': // Boolean not
				ret = eh_xtobool(eh_execute(node->opval->paras[0], context));
				ret.boolval = !ret.boolval;
				break;
			case T_GLOBAL: // global variable declaration
				eh_op_global(node->opval->paras[0]->stringval, context);
				break;
		/*
		 * Control flow
		 */
			case T_IF:
				if(eh_xtobool(eh_execute(node->opval->paras[0], context)).boolval)
					ret = eh_execute(node->opval->paras[1], context);
				else if(node->opval->nparas == 3)
					ret = eh_execute(node->opval->paras[2], context);
				break;
			case T_WHILE:
				ret = eh_op_while(node->opval->paras, context);
				break;
			case T_FOR:
				ret = eh_op_for(node->opval, context);
				break;
			case T_AS:
				ret = eh_op_as(node->opval, context);
				break;
			case T_SWITCH: // switch statements
				ret = eh_op_switch(node->opval->paras, context);
				// incremented in the eh_op_switch function
				inloop--;
				break;
			case T_GIVEN: // inline switch statements
				ret = eh_op_given(node->opval->paras, context);
				break;
		/*
		 * Miscellaneous
		 */
			case T_SEPARATOR:
				// if we're in an empty list
				if(node->opval->nparas == 0)
					return ret;
				// else execute both commands
				ret = eh_execute(node->opval->paras[0], context);
				if(returning || breaking || continuing)
					return ret;
				ret = eh_execute(node->opval->paras[1], context);
				break;
			case T_EXPRESSION: // wrapper for special case
				ret = eh_execute(node->opval->paras[0], context);
				break;
			case T_CALL: // call: execute argument and discard it
				eh_execute(node->opval->paras[0], context);
				break;
			case T_RET: // return from a function or the program
				ret = eh_execute(node->opval->paras[0], context);
				returning = true;
				break;
			case T_BREAK: // break out of a loop
				eh_op_break(node->opval, context);
				break;
			case T_CONTINUE: // continue in a loop
				eh_op_continue(node->opval, context);
				break;
		/*
		 * Object access
		 */
			case ':': // function call
				ret = eh_op_colon(node->opval->paras, context);
				break;
			case T_ACCESSOR: // array access, and similar stuff for other types
				ret = eh_op_accessor(node->opval->paras, context);
				break;
			case T_NEW: // object declaration
				ret = eh_op_new(eh_execute(node->opval->paras[0], context).stringval);
				break;
		/*
		 * Object definitions
		 */
			case T_FUNC: // function definition
				if(node->opval->nparas == 3)
					eh_op_declarefunc(node->opval->paras);
				else
					ret = eh_op_declareclosure(node->opval->paras);
				break;
			case T_CLASS: // class declaration
				eh_op_declareclass(node->opval->paras, context);
				break;
			case T_ATTRIBUTE: // class member attributes
				ret.type = attributestr_e;
				if(node->opval->nparas == 0) {
					// all zeroes
					ret.intval = 0;
				} else {
					// first execute first para
					ret = eh_execute(node->opval->paras[0], context);
					// then overwrite with attribute from second para
					switch(node->opval->paras[1]->attributeval) {
						case publica_e:
							ret.attributestrval.visibility = public_e;
							break;
						case privatea_e:
							ret.attributestrval.visibility = private_e;
							break;
						case statica_e:
							ret.attributestrval.isstatic = static_e;
							break;
						case consta_e:
							ret.attributestrval.isconst = const_e;
							break;
					}
				}
				break;
			case '[': // array declaration
				ret = eh_op_array(node->opval->paras[0], context);
				break;
		/*
		 * Binary operators
		 */
			case '=': // equality
				operand1 = eh_execute(node->opval->paras[0], context);
				operand2 = eh_execute(node->opval->paras[1], context);
				ret = eh_looseequals(operand1, operand2);
				break;
			case T_SE: // strict equality
				operand1 = eh_execute(node->opval->paras[0], context);
				operand2 = eh_execute(node->opval->paras[1], context);
				ret = eh_strictequals(operand1, operand2);
				break;
			case T_SNE: // strict non-equality
				operand1 = eh_execute(node->opval->paras[0], context);
				operand2 = eh_execute(node->opval->paras[1], context);
				ret = eh_strictequals(operand1, operand2);
				ret.boolval = !ret.boolval;
				break;
			EH_INTBOOL_CASE('>', >) // greater-than
			EH_INTBOOL_CASE('<', <) // lesser-than
			EH_INTBOOL_CASE(T_GE, >=) // greater-than or equal
			EH_INTBOOL_CASE(T_LE, <=) // lesser-than or equal
			EH_INTBOOL_CASE(T_NE, !=) // not equal
			// doing addition on two strings performs concatenation
			case '+':
				operand1 = eh_execute(node->opval->paras[0], context);
				operand2 = eh_execute(node->opval->paras[1], context);
				ret = eh_op_plus(operand1, operand2);
				break;
			EH_FLOATINT_CASE('-', -) // subtraction
			EH_FLOATINT_CASE('*', *) // multiplication
			EH_FLOATINT_CASE('/', /) // division
			EH_INT_CASE('%', %) // modulo
			EH_INT_CASE('&', &) // bitwise AND
			EH_INT_CASE('^', ^) // bitwise XOR
			EH_INT_CASE('|', |) // bitwise OR
			case T_AND: // AND; use short-circuit operation
				operand1 = eh_xtobool(eh_execute(node->opval->paras[0], context));
				if(!operand1.boolval)
					ret = operand1;
				else
					ret = eh_xtobool(eh_execute(node->opval->paras[1], context));
				break;
			case T_OR: // OR; use short-circuit operation
				operand1 = eh_xtobool(eh_execute(node->opval->paras[0], context));
				if(operand1.boolval)
					ret = operand1;
				else
					ret = eh_xtobool(eh_execute(node->opval->paras[1], context));
				break;
			case T_XOR:
				operand1 = eh_xtobool(eh_execute(node->opval->paras[0], context));
				operand2 = eh_xtobool(eh_execute(node->opval->paras[1], context));
				ret.type = bool_e;
				if((operand1.boolval && operand2.boolval) || (!operand1.boolval && !operand2.boolval))
					ret.boolval = false;
				else
					ret.boolval = true;
				break;
		/*
		 * Variable manipulation
		 */
			case T_LVALUE_GET:
			case T_LVALUE_SET:
				ret = eh_op_lvalue(node->opval, context);
				break;
			case T_RANGE:
				// Attempt to cast operands to integers; if this does not work,
				// return NULL. No need to yell, since eh_xtoi already does
				// that.
				operand1 = eh_xtoint(eh_execute(node->opval->paras[0], context));
				if(operand1.type == null_e)
					break;
				operand2 = eh_xtoint(eh_execute(node->opval->paras[1], context));
				if(operand2.type == null_e)
					break;
				ret = eh_make_range(operand1.intval, operand2.intval);
				break;
			case T_SET:
				eh_op_set(node->opval->paras, context);
				break;
			case T_MINMIN:
				operand1 = eh_execute(node->opval->paras[0], context);
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
				operand1 = eh_execute(node->opval->paras[0], context);
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
				ret = eh_execute(node->opval->paras[0], context);
				if(ret.type != reference_e)
					eh_error("Unable to create reference", eerror_e);
				break;
			case '$': // variable dereference
				ret = eh_op_dollar(node->opval->paras[0], context);
				break;
		/*
		 * Commands
		 */
			case T_COMMAND:
				// name of command to be executed
				ret = eh_op_command(
					eh_execute(node->opval->paras[0], context).stringval,
					node->opval->paras[1],
					context
				);
				break;
			default:
				eh_error_int("Unexpected opcode", node->opval->op, efatal_e);
				break;
		}
	else
		ret = *node;
	return ret;
}
/*
 * Opnode execution helpers
 */
void print_retval(const ehretval_t ret) {
	switch(ret.type) {
		case string_e:
			printf("%s", ret.stringval);
			break;
		case int_e:
			printf("%d", ret.intval);
			break;
		case bool_e:
			if(ret.boolval)
				printf("(true)");
			else
				printf("(false)");
			break;
		case null_e:
			printf("(null)");
			break;
		case float_e:
			printf("%f", ret.floatval);
			break;
		case range_e:
			printf("%d to %d", ret.rangeval->min, ret.rangeval->max);
			break;
		default:
			eh_error_type("echo operator", ret.type, enotice_e);
			break;
	}
	return;
}
ehretval_t eh_count(const ehretval_t in) {
	ehretval_t ret;
	ret.type = int_e;
	switch(in.type) {
		case int_e:
			ret.intval = sizeof(int) * 8;
			break;
		case float_e:
			ret.intval = sizeof(float) * 8;
			break;
		case string_e:
			ret.intval = strlen(in.stringval);
			break;
		case array_e:
			ret.intval = array_count(in.arrayval);
			break;
		case null_e:
			ret.intval = 0;
			break;
		case bool_e:
			ret.intval = 0;
			break;
		default:
			eh_error_type("count operator", in.type, eerror_e);
			ret.type = null_e;
			break;
	}
	return ret;
}
ehretval_t eh_op_tilde(ehretval_t in) {
	// no const argument because it's modified below
	ehretval_t ret;
	switch(in.type) {
		// bitwise negation of a bool is just normal negation
		case bool_e:
			ret.type = bool_e;
			ret.boolval = !in.boolval;
			break;
		// else try to cast to int
		default:
			in = eh_xtoint(in);
			if(in.type != int_e) {
				eh_error_type("bitwise negation", in.type, eerror_e);
				ret.type = null_e;
				return ret;
			}
			// fall through to int case
		case int_e:
			ret.type = int_e;
			ret.intval = ~in.intval;
			break;
	}
	return ret;
}
ehretval_t eh_op_uminus(ehretval_t in) {
	ehretval_t ret;
	switch(in.type) {
		// negation
		case bool_e:
			ret.type = bool_e;
			ret.boolval = !in.boolval;
			break;
		case float_e:
			ret.type = float_e;
			ret.floatval = -in.floatval;
		default:
			in = eh_xtoint(in);
			if(in.type != int_e) {
				eh_error_type("negation", in.type, eerror_e);
				ret.type = null_e;
				return ret;
			}
			// fall through to int case
		case int_e:
			ret.type = int_e;
			ret.intval = -in.intval;
			break;
	}
	return ret;
}
ehretval_t eh_op_plus(ehretval_t operand1, ehretval_t operand2) {
	ehretval_t ret;
	if(operand1.type == string_e && operand2.type == string_e) {
		// concatenate them
		ret.type = string_e;
		size_t len1, len2;
		len1 = strlen(operand1.stringval);
		len2 = strlen(operand2.stringval);
		ret.stringval = (char *) Malloc(len1 + len2 + 1);
		strcpy(ret.stringval, operand1.stringval);
		strcpy(ret.stringval + len1, operand2.stringval);
	}
	else if(operand1.type == float_e && operand2.type == float_e) {
		ret.type = float_e;
		ret.floatval = operand1.floatval + operand2.floatval;
	}
	else {
		operand1 = eh_xtoint(operand1);
		operand2 = eh_xtoint(operand2);
		if(operand1.type == int_e && operand2.type == int_e) {
			ret.type = int_e;
			ret.intval = operand1.intval + operand2.intval;
		}
		else
			ret.type = null_e;
	}
	return ret;
}
void eh_op_global(const char *name, ehcontext_t context) {
	ehvar_t *globalvar;
	ehvar_t *newvar;
	globalvar = get_variable(name, 0, context);
	if(globalvar == NULL) {
		eh_error_unknown("global variable", name, enotice_e);
		return;
	}
	newvar = (ehvar_t *) Malloc(sizeof(ehvar_t));
	newvar->name = name;
	newvar->scope = scope;
	newvar->value.type = reference_e;
	newvar->value.referenceval = &globalvar->value;
	insert_variable(newvar);
	return;
}
ehretval_t eh_op_command(const char *name, ehretval_t *node, ehcontext_t context) {
	ehretval_t index_r, value_r;
	ehvar_t **paras;
	ehretval_t *node2;
	// count for simple parameters
	int count = 0;
	// we're making an array of parameters
	paras = (ehvar_t **) Calloc(VARTABLE_S, sizeof(ehvar_t));
	// loop through the paras given
	while(node->opval->nparas != 0) {
		node2 = node->opval->paras[0];
		if(node2->type == op_e) {
			switch(node2->opval->op) {
				case T_SHORTPARA:
					// short paras: set each letter to true
					node2 = node2->opval->paras[0];
					for(int i = 0, len = strlen(node2->stringval); i < len; i++) {
						index_r.type = string_e;
						index_r.stringval = (char *) Malloc(2);
						index_r.stringval[0] = node2->stringval[i];
						index_r.stringval[1] = '\0';
						value_r.type = bool_e;
						value_r.boolval = true;
						array_insert_retval(paras, index_r, value_r);
					}
					break;
				case T_LONGPARA:
					// long-form paras
					if(node2->opval->nparas == 1) {
						value_r.type = bool_e;
						value_r.boolval = true;
						array_insert_retval(
							paras,
							eh_execute(node2->opval->paras[0], context),
							value_r
						);
					}
					else {
						array_insert_retval(
							paras,
							eh_execute(node2->opval->paras[0], context),
							eh_execute(node2->opval->paras[1], context)
						);
					}
					break;
				case '>':
					index_r.type = string_e;
					index_r.stringval = (char *) Malloc(sizeof(">"));
					strcpy(index_r.stringval, ">");
					// output redirector
					array_insert_retval(
						paras,
						index_r,
						eh_execute(node2->opval->paras[0], context)
					);
					break;
				case '}':
					index_r.type = string_e;
					index_r.stringval = (char *) Malloc(sizeof("}"));
					strcpy(index_r.stringval, "}");
					// output redirector
					array_insert_retval(
						paras,
						index_r,
						eh_execute(node2->opval->paras[0], context)
					);
					break;
				default: // non-named parameters with an expression
					// non-named parameters
					index_r.type = int_e;
					index_r.intval = count;
					value_r = eh_execute(node2, context);
					array_insert_retval(paras, index_r, value_r);
					count++;
					break;
			}
		}
		else {
			// non-named parameters
			index_r.type = int_e;
			index_r.intval = count;
			value_r = eh_execute(node2, context);
			array_insert_retval(paras, index_r, value_r);
			count++;
		}
		node = node->opval->paras[1];
	}
	ehretval_t ret = interpreter->execute_cmd(name, paras);
	// we're not returning anymore
	returning = false;
	return ret;
}
ehretval_t eh_op_for(opnode_t *op, ehcontext_t context) {
	ehretval_t ret, count_r;
	inloop++;
	breaking = 0;
	ehrange_t range;

	// initialize return value
	ret.type = null_e;
	// get the count
	count_r = eh_execute(op->paras[0], context);
	if(count_r.type == range_e) {
		range = *count_r.rangeval;
	}
	else {
		count_r = eh_xtoint(count_r);
		if(count_r.type != int_e) {
			eh_error_type("count", count_r.type, eerror_e);
			return ret;
		}
		range.min = 0;
		range.max = count_r.intval - 1;
	}
	if(op->nparas == 2) {
		// "for 5; do stuff; endfor" construct
		for(int i = range.min; i <= range.max; i++) {
			ret = eh_execute(op->paras[1], context);
			LOOPCHECKS;
		}
	}
	else {
		// "for 5 count i; do stuff; endfor" construct
		char *name = op->paras[1]->stringval;
		ehvar_t *var = get_variable(name, scope, context);
		// variable is not yet set, so set it
		if(var == NULL) {
			var = (ehvar_t *) Malloc(sizeof(ehvar_t));
			var->name = op->paras[1]->stringval;
			var->scope = scope;
			insert_variable(var);
		}
		// count variable always gets to be an int
		var->value.type = int_e;
		for(var->value.intval = range.min; var->value.intval <= range.max; var->value.intval++) {
			ret = eh_execute(op->paras[2], context);
			LOOPCHECKS;
		}
	}
	inloop--;
	return ret;
}
ehretval_t eh_op_while(ehretval_t **paras, ehcontext_t context) {
	ehretval_t ret;
	ret.type = null_e;
	inloop++;
	breaking = 0;
	while(eh_xtobool(eh_execute(paras[0], context)).boolval) {
		ret = eh_execute(paras[1], context);
		LOOPCHECKS;
	}
	inloop--;
	return ret;
}
ehretval_t eh_op_as(opnode_t *op, ehcontext_t context) {
	ehretval_t ret;
	ret.type = null_e;

	// get the object to be looped through and check its type
	ehretval_t object = eh_execute(op->paras[0], context);
	if(object.type != array_e && object.type != object_e) {
		eh_error_type("for ... as operator", object.type, enotice_e);
		return ret;
	}
	// increment loop count
	inloop++;
	// establish variables
	char *membername;
	ehvar_t *membervar;
	char *indexname;
	ehvar_t *indexvar;
	ehretval_t *code;
	// no index
	if(op->nparas == 3) {
		membername = op->paras[1]->stringval;
		indexname = NULL;
		code = op->paras[2];
	}
	// with index
	else {
		indexname = op->paras[1]->stringval;
		membername = op->paras[2]->stringval;
		code = op->paras[3];
	}
	// create variables
	membervar = get_variable(membername, scope, context);
	if(membervar == NULL) {
		membervar = (ehvar_t *) Malloc(sizeof(ehvar_t));
		membervar->name = membername;
		membervar->scope = scope;
		insert_variable(membervar);
	}
	if(indexname != NULL) {
		indexvar = get_variable(indexname, scope, context);
		if(indexvar == NULL) {
			indexvar = (ehvar_t *) Malloc(sizeof(ehvar_t));
			indexvar->name = indexname;
			indexvar->scope = scope;
			insert_variable(indexvar);
		}
	}
	if(object.type == object_e) {
		// object index is always a string
		indexvar->value.type = string_e;
		ehvar_t **members = object.objectval->members;
		// check whether we're allowed to access private things
		const bool doprivate = ehcontext_compare(object.objectval, context);
		for(int i = 0; i < VARTABLE_S; i++) {
			for(ehvar_t *currmember = members[i]; currmember != NULL; currmember = currmember->next) {
				// ignore private
				if(!doprivate && currmember->attribute.visibility == private_e)
					continue;
				if(currmember->attribute.isconst == const_e) {
					membervar->value.type = creference_e;
					membervar->value.referenceval = &currmember->value;
				}
				else
					membervar->value = currmember->value;
				if(indexname) {
					// need the strdup here because currmember->name is const
					// and a string_e is not. Perhaps solve this instead by
					// creating a new cstring_e type?
					indexvar->value.stringval = strdup(currmember->name);
				}
				ret = eh_execute(code, context);
				LOOPCHECKS;
			}
		}
	}
	else {
		// arrays
		ehvar_t **members = object.arrayval;
		for(int i = 0; i < VARTABLE_S; i++) {
			for(ehvar_t *currmember = members[i]; currmember != NULL; currmember = currmember->next) {
				membervar->value = currmember->value;
				if(indexname) {
					indexvar->value.type = currmember->indextype;
					if(currmember->indextype == string_e)
						indexvar->value.stringval = strdup(currmember->name);
					else
						indexvar->value.intval = currmember->index;
				}
				ret = eh_execute(code, context);
				LOOPCHECKS;
			}
		}
	}
	inloop--;
	return ret;
}
ehretval_t eh_op_new(const char *name) {
	ehretval_t ret;

	ehclass_t *classobj = get_class(name);
	if(classobj == NULL) {
		eh_error_unknown("class", name, efatal_e);
		ret.type = null_e;
		return ret;
	}
	ret.type = object_e;
	ret.objectval = (ehobj_t *) Malloc(sizeof(ehobj_t));
	ret.objectval->classname = name;
	ret.objectval->members = (ehvar_t **) Calloc(VARTABLE_S, sizeof(ehvar_t *));
	for(int i = 0; i < VARTABLE_S; i++) {
		for(ehvar_t *m = classobj->obj.members[i]; m != NULL; m = m->next) {
			class_copy_member(ret.objectval, m, i);
		}
	}
	return ret;
}
void eh_op_break(opnode_t *op, ehcontext_t context) {
	ehretval_t level;
	if(op->nparas == 0) {
		level.type = int_e;
		level.intval = 1;
	}
	else {
		level = eh_xtoint(eh_execute(op->paras[0], context));
		if(level.type != int_e)
			return;
	}
	// break as many levels as specified by the argument
	if(level.intval > inloop) {
		eh_error_looplevels("Cannot break", level.intval);
		return;
	}
	breaking = level.intval;
	return;
}
void eh_op_continue(opnode_t *op, ehcontext_t context) {
	ehretval_t level;
	if(op->nparas == 0) {
		level.type = int_e;
		level.intval = 1;
	}
	else {
		level = eh_xtoint(eh_execute(op->paras[0], context));
		if(level.type != int_e)
			return;
	}
	// break as many levels as specified by the argument
	if(level.intval > inloop) {
		eh_error_looplevels("Cannot continue", level.intval);
		return;
	}
	continuing = level.intval;
	return;
}
ehretval_t eh_op_array(ehretval_t *node, ehcontext_t context) {
	ehretval_t ret;
	ehretval_t *node2;
	ret.type = array_e;
	ret.arrayval = (ehvar_t **) Calloc(VARTABLE_S, sizeof(ehvar_t *));
	// need to count array members first, because they are reversed in our node.
	// That's not necessary with functions (where the situation is analogous), because the reversals that happen when parsing the prototype argument list and parsing the argument list in a call cancel each other out.
	int count = 0;
	node2 = node;
	while(node2->opval->nparas != 0) {
		count++;
		node2 = node2->opval->paras[0];
	}
	node2 = node;
	while(node2->opval->nparas != 0) {
		array_insert(ret.arrayval, node2->opval->paras[1], --count, context);
		node2 = node2->opval->paras[0];
	}
	return ret;
}
void eh_op_declarefunc(ehretval_t **paras) {
	char *name;
	ehfunc_t *func;

	name = paras[0]->stringval;
	func = get_function(name);
	// function definition
	if(func != NULL) {
		eh_error_redefine("function", name, efatal_e);
		return;
	}
	func = (ehfunc_t *) Malloc(sizeof(ehfunc_t));
	func->name = name;
	// determine argcount
	make_arglist(&func->f.argcount, &func->f.args, paras[1]);
	func->f.code = paras[2];
	func->f.type = user_e;
	insert_function(func);
	return;
}
ehretval_t eh_op_declareclosure(ehretval_t **paras) {
	ehretval_t ret;
	ret.type = func_e;
	ret.funcval = (ehfm_t *) Malloc(sizeof(ehfm_t));
	ret.funcval->type = user_e;
	make_arglist(&ret.funcval->argcount, &ret.funcval->args, paras[0]);
	ret.funcval->code = paras[1];
	return ret;
}
void eh_op_declareclass(ehretval_t **paras, ehcontext_t context) {
	ehretval_t classname_r;
	ehclass_t *classobj;

	classname_r = eh_execute(paras[0], context);
	classobj = get_class(classname_r.stringval);
	if(classobj != NULL) {
		eh_error_redefine("class", classname_r.stringval, efatal_e);
		return;
	}
	classobj = (ehclass_t *) Malloc(sizeof(ehclass_t));
	classobj->obj.classname = classname_r.stringval;
	classobj->obj.members = (ehvar_t **) Calloc(VARTABLE_S, sizeof(ehvar_t *));
	// insert class members
	ehretval_t *node = paras[1];
	while(node != NULL) {
		if(node->type == op_e && node->opval->op == ',') {
			class_insert(classobj->obj.members, node->opval->paras[0], context);
			node = node->opval->paras[1];
		}
		else {
			class_insert(classobj->obj.members, node, context);
			break;
		}
	}
	// insert this pointer
	memberattribute_t thisattributes;
	thisattributes.visibility = private_e;
	thisattributes.isstatic = nonstatic_e;
	thisattributes.isconst = const_e;
	ehretval_t thisvalue;
	thisvalue.type = object_e;
	thisvalue.objectval = &(classobj->obj);
	class_insert_retval(classobj->obj.members, "this", thisattributes, thisvalue);
	insert_class(classobj);
	return;
}
ehretval_t eh_op_switch(ehretval_t **paras, ehcontext_t context) {
	ehretval_t switchvar, casevar, ret;
	ehretval_t *node;
	opnode_t *op;

	// because we use continue, we'll pretend this is a loop
	inloop++;

	// switch variable
	switchvar = eh_execute(paras[0], context);
	for(node = paras[1]; node->opval->nparas != 0; node = node->opval->paras[1]) {
		op = node->opval->paras[0]->opval;
		// execute default
		if(op->nparas == 1) {
			ret = eh_execute(op->paras[0], context);
		}
		else {
			casevar = eh_execute(op->paras[0], context);
			ehretval_t decider;
			// try to call function
			if(casevar.type == func_e) {
				decider = call_function_args(casevar.funcval, context, newcontext, 1, &switchvar);
				if(decider.type != bool_e) {
					eh_error("Switch case method does not return bool", eerror_e);
					ret.type = null_e;
					return ret;
				}
			}
			else
				decider = eh_looseequals(switchvar, casevar);
			// apply the decider
			if(decider.boolval)
				ret = eh_execute(op->paras[1], context);
			else
				continue;
		}
		// check whether we need to leave
		if(returning)
			return ret;
		else if(breaking) {
			breaking--;
			return ret;
		}
		else if(continuing) {
			// if continuing == 1, then continue
			continuing--;
			// so if continuing now > 0, leave the switch
			if(continuing)
				return ret;
		}
		else
			return ret;
	}
	ret.type = null_e;
	return ret;
}
ehretval_t eh_op_given(ehretval_t **paras, ehcontext_t context) {
	ehretval_t switchvar, casevar, ret;
	ehretval_t *node;
	opnode_t *op;

	// switch variable
	switchvar = eh_execute(paras[0], context);
	node = paras[1];
	while(node->opval->nparas != 0) {
		op = node->opval->paras[0]->opval;
		// execute default
		if(op->nparas == 1)
			return eh_execute(op->paras[0], context);
		casevar = eh_execute(op->paras[0], context);
		ehretval_t decider;
		if(casevar.type == func_e) {
			decider = call_function_args(casevar.funcval, context, newcontext, 1, &switchvar);
			if(decider.type != bool_e) {
				eh_error("Given case method does not return bool", eerror_e);
				ret.type = null_e;
				return ret;
			}
		}
		else
			decider = eh_looseequals(switchvar, casevar);
		if(decider.boolval)
			return eh_execute(op->paras[1], context);
		node = node->opval->paras[1];
	}
	ret.type = null_e;
	return ret;
}
ehretval_t eh_op_colon(ehretval_t **paras, ehcontext_t context) {
	ehretval_t function;
	ehretval_t ret;
	ret.type = null_e;

	function = eh_execute(paras[0], context);
	// operand1 will be either a string (indicating a normal function call) or a func_e (indicating a method or closure call)
	if(function.type == string_e) {
		ehfunc_t *func = get_function(function.stringval);
		if(func == NULL) {
			eh_error_unknown("function", function.stringval, efatal_e);
			return ret;
		}
		ret = call_function(&func->f, paras[1], context, context);
	}
	else if(function.type == func_e) {
		ret = call_function(function.funcval, paras[1], context, newcontext);
	}
	// ignore null_e, because private method calls otherwise get confused
	else if(function.type != null_e) {
		eh_error_type("function call", function.type, eerror_e);
	}
	return ret;
}
ehretval_t eh_op_lvalue(opnode_t *op, ehcontext_t context) {
	/*
	 * Get an lvalue. This function normally returns an ehretval_t of type
	 * reference_e: a pointer to an ehretval_t that can be modified by the
	 * calling code.
	 *
	 * Because of special needs of calling code, this case actually returns
	 * useful data in the second field of the ehretval_t struct if its type is
	 * null_e: either NULL (if referring to a non-existing  variable) and 0x1
	 * (if referring to a member of a non-existent variable).
	 *
	 * Otherwise, it returns an attribute_e with a pointer to the ehretval_t of
	 * the variable referred to, so that T_SET can do its bitwise magic with
	 * ints and similar stuff.
	 */
	ehretval_t ret, basevar, index;
	ehvar_t *var, *member;

	basevar = eh_execute(op->paras[0], context);
	ret.type = null_e;
	ret.referenceval = NULL;
	switch(op->nparas) {
		case 1:
			var = get_variable(basevar.stringval, scope, context);
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
			switch(op->paras[1]->accessorval) {
				case arrow_e:
					var = get_variable(basevar.stringval, scope, context);
					if(var == NULL) {
						eh_error("Cannot access member of non-existing variable", eerror_e);
						ret.referenceval = (ehretval_t *) 0x1;
					}
					if(var->value.type == array_e) {
						index = eh_execute(op->paras[2], context);
						member = array_getmember(var->value.arrayval, index);
						// if there is no member yet and we are
						// setting, insert it with a null value
						if(member == NULL) {
							if(op->op == T_LVALUE_SET) {
								member = array_insert_retval(
									var->value.arrayval,
									index,
									ret
								);
								ret.type = reference_e;
								ret.referenceval = &member->value;
							}
							// else use default return value
						}
						else {
							ret.type = reference_e;
							ret.referenceval = &member->value;
						}
					}
					else {
						ret.type = attribute_e;
						ret.referenceval = &var->value;
					}
					break;
				case dot_e:
					ret = object_access(basevar, op->paras[2], context, op->op);
					break;
				case doublecolon_e:
					ret = colon_access(basevar, op->paras[2], context, op->op);
					break;
				default:
					eh_error("Unsupported accessor", efatal_e);
				break;
			}
			break;
	}
	return ret;
}
ehretval_t eh_op_dollar(ehretval_t *node, ehcontext_t context) {
	ehretval_t ret, index;
	ret = eh_execute(node, context);
	if(ret.type == null_e)
		return ret;
	else if(ret.type == attribute_e) {
		// get operands
		index = eh_execute(node->opval->paras[2], context);
		if(index.type == reference_e)
			ret = *ret.referenceval;
		switch(ret.referenceval->type) {
			case int_e:
				ret = int_arrow_get(*ret.referenceval, index);
				break;
			case string_e:
				ret = string_arrow_get(*ret.referenceval, index);
				break;
			case range_e:
				ret = range_arrow_get(*ret.referenceval, index);
				break;
			default:
				eh_error_type("array-type dereference", ret.referenceval->type, eerror_e);
				break;
		}
	}
	else while(ret.type == reference_e || ret.type == creference_e)
		ret = *ret.referenceval;
	return ret;
}
void eh_op_set(ehretval_t **paras, ehcontext_t context) {
	ehretval_t lvalue = eh_execute(paras[0], context);
	ehretval_t rvalue = eh_execute(paras[1], context);
	ehretval_t index;
	if(lvalue.type == null_e) {
		if(lvalue.referenceval == NULL) {
			// set new variable
			ehvar_t *var = (ehvar_t *) Malloc(sizeof(ehvar_t));
			var->name = paras[0]->opval->paras[0]->stringval;
			var->scope = scope;
			var->value = rvalue;
			insert_variable(var);
		}
		// else do nothing; T_LVALUE will already have complained
	}
	else if(lvalue.type == attribute_e) {
		// lvalue is a pointer to the variable modified, rvalue is the value set to, index is the index
		index = eh_execute(paras[0]->opval->paras[2], context);
		switch(lvalue.referenceval->type) {
			case int_e:
				int_arrow_set(lvalue, index, rvalue);
				break;
			case string_e:
				string_arrow_set(lvalue, index, rvalue);
				break;
			case range_e:
				range_arrow_set(lvalue, index, rvalue);
				break;
			default:
				eh_error_type("array access", lvalue.referenceval->type, eerror_e);
				break;
		}
	}
	else {
		while(lvalue.type == reference_e && lvalue.referenceval->type == reference_e)
			lvalue = *(lvalue.referenceval);
		// set variable, unless it is const
		if(lvalue.type == creference_e)
			eh_error("Attempt to write to constant variable", eerror_e);
		/*
		 * Without this check, the following code creates an
		 * infinite loop:
			$ foo = 3
			$ bar = &foo
			$ bar = &foo
			echo $foo
		 * That is because the third line sets foo to its own
		 * address.
		 */
		else if(rvalue.type == reference_e && lvalue.referenceval == rvalue.referenceval)
			eh_error("Circular reference", eerror_e);
		else
			*lvalue.referenceval = rvalue;
	}
	return;
}
ehretval_t eh_op_accessor(ehretval_t **paras, ehcontext_t context) {
	// this only gets executed for array-type int and string access
	ehretval_t ret;
	ret.type = null_e;
	if(paras[1]->accessorval == arrow_e) {
		// "array" access
		ehretval_t basevar = eh_execute(paras[0], context);
		ehretval_t index = eh_execute(paras[2], context);
		switch(basevar.type) {
			case int_e:
				ret = int_arrow_get(basevar, index);
				break;
			case string_e:
				ret = string_arrow_get(basevar, index);
				break;
			case array_e:
				// array access to an array works as expected.
				ret = array_get(basevar.arrayval, index);
				break;
			default:
				eh_error_type("array access", basevar.type, eerror_e);
				break;
		}
	}
	else
		eh_error("Unsupported accessor", efatal_e);
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
ehvar_t *get_variable(const char *name, int scope, ehcontext_t context) {
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
	// else try the object
	if(context) {
		if((currvar = class_getmember(context, name, context))) {
			return currvar;
		}
	}
	return NULL;
}
void remove_variable(const char *name, int scope) {
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
	unsigned int vhash = hash(func->name, HASH_INITVAL);
	func->next = functable[vhash];
	functable[vhash] = func;
	return true;
}
ehfunc_t *get_function(const char *name) {
	for(ehfunc_t *currfunc = functable[hash(name, HASH_INITVAL)]; 
	  currfunc != NULL; currfunc = currfunc->next) {
		if(strcmp(currfunc->name, name) == 0) {
			return currfunc;
		}
	}
	return NULL;
}
static void make_arglist(int *argcount, eharg_t **arglist, ehretval_t *node) {
	*argcount = 0;
	// traverse linked list to determine argument count
	int currarg = 0;

	for(ehretval_t *tmp = node; tmp->opval->nparas != 0; 
	  tmp = tmp->opval->paras[0]) {
		currarg++;
	}
	*argcount = currarg;
	// if there are no arguments, the arglist can be NULL
	if(currarg) {
		*arglist = (eharg_t *) Malloc(currarg * sizeof(eharg_t));
	} else {
		*arglist = NULL;
	}
	// add arguments to arglist
	currarg = 0;
	for(ehretval_t *tmp = node; tmp->opval->nparas != 0; 
	  tmp = tmp->opval->paras[0]) {
		(*arglist)[currarg].name = tmp->opval->paras[1]->stringval;
		currarg++;
	}
}
ehretval_t call_function(ehfm_t *f, ehretval_t *args, ehcontext_t context, ehcontext_t newcontext) {
	ehretval_t ret;

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
		ehvar_t *var = (ehvar_t *) Malloc(sizeof(ehvar_t));
		var->name = f->args[i].name;
		var->scope = scope + 1;
		insert_variable(var);
		i++;
		if(i > f->argcount) {
			eh_error_argcount(f->argcount, i);
			return ret;
		}
		var->value = eh_execute(args->opval->paras[1], context);
		args = args->opval->paras[0];
	}
	// functions get their own scope (not incremented before because execution of arguments needs parent scope)
	scope++;
	if(f->argcount != i) {
		eh_error_argcount(f->argcount, i);
		return ret;
	}
	// set new context (only useful for methods)
	ret = eh_execute(f->code, newcontext);
	returning = false;
	for(i = 0; i < f->argcount; i++) {
		remove_variable(f->args[i].name, scope);
	}
	scope--;
	return ret;
}
ehretval_t call_function_args(const ehfm_t *const f, const ehcontext_t context, const ehcontext_t newcontext, const int nargs, const ehretval_t *const args) {
	ehretval_t ret;

	ret.type = null_e;
	if(f->type == lib_e) {
		// library function not supported here for now
		eh_error("call_function_args does not support library functions", 
			efatal_e);
		return ret;
	}
	// check parameter count
	if(nargs != f->argcount) {
		eh_error_argcount(f->argcount, nargs);
		return ret;
	}
	// set parameters as necessary
	for(int i = 0; i < nargs; i++) {
		ehvar_t *var = (ehvar_t *) Malloc(sizeof(ehvar_t));
		var->name = f->args[i].name;
		var->scope = scope + 1;
		var->value = eh_execute(&args[i], context);
		insert_variable(var);
	}
	// functions get their own scope (not incremented before because execution 
	// of arguments needs parent scope)
	scope++;
	// set new context (only useful for methods)
	ret = eh_execute(f->code, newcontext);
	returning = false;
	for(int i = 0; i < nargs; i++) {
		remove_variable(f->args[i].name, scope);
	}
	scope--;
	return ret;
}
/*
 * Classes
 */
void insert_class(ehclass_t *classobj) {
	unsigned int vhash = hash(classobj->obj.classname, HASH_INITVAL);
	classobj->next = classtable[vhash];
	classtable[vhash] = classobj;
	return;
}
ehclass_t *get_class(const char *name) {
	for(ehclass_t *currclass = classtable[hash(name, HASH_INITVAL)]; 
	  currclass != NULL; currclass = currclass->next) {
		if(strcmp(currclass->obj.classname, name) == 0) {
			return currclass;
		}
	}
	return NULL;
}
void class_copy_member(ehobj_t *classobj, ehvar_t *classmember, int i) {
	ehvar_t *newmember = (ehvar_t *) Malloc(sizeof(ehvar_t));
	// copy the whole thing over
	*newmember = *classmember;
	// modify this pointer
	if(!strcmp(newmember->name, "this")) {
		newmember->value.type = object_e;
		newmember->value.objectval = classobj;
	} else if(classmember->attribute.isstatic == static_e) {
		// handle static
		newmember->value.type = reference_e;
		newmember->value.referenceval = &classmember->value;
	}
	newmember->next = classobj->members[i];
	classobj->members[i] = newmember;
	return;
}
void class_insert(ehvar_t **classarr, ehretval_t *in, ehcontext_t context) {
	// insert a member into a class
	ehretval_t value;

	// rely on standard layout of the input ehretval_t
	memberattribute_t attribute = eh_execute(in->opval->paras[0], 
		context).attributestrval;
	char *name = in->opval->paras[1]->stringval;

	// decide what we got
	switch(in->opval->nparas) {
		case 2: // non-set property: null
			value.type = null_e;
			break;
		case 3: // set property
			value = eh_execute(in->opval->paras[2], context);
			break;
		case 4: // method
			value.type = func_e;
			value.funcval = (ehfm_t *) Malloc(sizeof(ehfm_t));
			value.funcval->code = in->opval->paras[3];
			make_arglist(&value.funcval->argcount, &value.funcval->args, 
				in->opval->paras[2]);
			break;
	}
	class_insert_retval(classarr, name, attribute, value);
}
ehvar_t *class_insert_retval(
	ehvar_t **classarr,
	const char *name,
	memberattribute_t attribute,
	ehretval_t value
) {
	// insert a member into a class

	ehvar_t *member = (ehvar_t *) Malloc(sizeof(ehvar_t));
	// rely on standard layout of the input ehretval_t
	member->attribute = attribute;
	member->name = name;
	member->value = value;

	// insert into hash table
	unsigned int vhash = hash(member->name, 0);
	member->next = classarr[vhash];
	classarr[vhash] = member;
	return member;
}
ehvar_t *class_getmember(ehobj_t *classobj, const char *name, ehcontext_t context) {
	for(ehvar_t *curr = classobj->members[hash(name, 0)]; curr != NULL; 
	  curr = curr->next) {
		if(!strcmp(curr->name, name)) {
			// we found it; now check visibility
			switch(curr->attribute.visibility) {
				case public_e:
					return curr;
				case private_e:
					// check context
					return ehcontext_compare(classobj, context) ? curr : NULL;
			}
		}
	}
	return NULL;
}
ehretval_t class_get(ehobj_t *classobj, const char *name, ehcontext_t context) {
	ehretval_t ret;

	ehvar_t *curr = class_getmember(classobj, name, context);
	if(curr == NULL) {
		ret.type = null_e;
	} else {
		ret = curr->value;
	}
	return ret;
}
ehretval_t object_access(
	ehretval_t operand1,
	ehretval_t *index,
	ehcontext_t context,
	int token
) {
	ehretval_t label, ret;
	ehvar_t *var;
	ehvar_t *classmember;
	ehobj_t *object;
	memberattribute_t attribute;

	// default value
	ret.type = null_e;

	label = eh_execute(index, context);
	if(label.type != string_e) {
		eh_error_type("object member label", label.type, eerror_e);
		return ret;
	}

	var = get_variable(operand1.stringval, scope, context);
	if(var->value.type != object_e) {
		eh_error_type("object access", var->value.type, eerror_e);
		return ret;
	}
	object = var->value.objectval;

	classmember = class_getmember(object, label.stringval, context);
	if(classmember == NULL) {
		// add new member if we're setting
		if(token == T_LVALUE_SET) {
			// default is public, non-static, non-constant
			attribute.visibility = public_e;
			attribute.isstatic = nonstatic_e;
			attribute.isconst = nonconst_e;
			classmember = class_insert_retval(
				object->members,
				label.stringval,
				attribute,
				ret
			);
		} else {
			eh_error_unknown("object member", label.stringval, eerror_e);
			return ret;
		}
	}
	// respect const specifier
	if(classmember->attribute.isconst == const_e) {
		ret.type = creference_e;
	} else {
		ret.type = reference_e;
	}
	ret.referenceval = &classmember->value;
	newcontext = object;
	return ret;
}
ehretval_t colon_access(
	ehretval_t operand1,
	ehretval_t *index,
	ehcontext_t context,
	int token
) {
	ehretval_t ret, label;
	memberattribute_t attribute;
	ret.type = null_e;
	ret.referenceval = NULL;

	label = eh_execute(index, context);
	if(label.type != string_e) {
		eh_error_type("object member label", label.type, eerror_e);
		return ret;
	}

	if(operand1.type != string_e) {
		eh_error_type("class access", operand1.type, efatal_e);
		return ret;
	}
	ehclass_t *classobj = get_class(operand1.stringval);
	if(!classobj) {
		eh_error_unknown("class", operand1.stringval, efatal_e);
		return ret;
	}
	ehvar_t *member = class_getmember(&classobj->obj, label.stringval, context);
	if(!member) {
		// add new member if we're setting
		if(token == T_LVALUE_SET) {
			// default is public, non-static, non-constant
			attribute.visibility = public_e;
			attribute.isstatic = nonstatic_e;
			attribute.isconst = nonconst_e;
			member = class_insert_retval(
				classobj->obj.members,
				label.stringval,
				attribute,
				ret
			);
		}
		else {
			eh_error_unknown("object member", label.stringval, eerror_e);
			return ret;
		}
	}
	// respect const specifier
	if(member->attribute.isconst == const_e) {
		ret.type = creference_e;
	} else {
		ret.type = reference_e;
	}
	ret.referenceval = &member->value;
	newcontext = &classobj->obj;
	return ret;
}
bool ehcontext_compare(const ehcontext_t lock, const ehcontext_t key) {
	// in global context, we never have access to private stuff
	if(key == NULL)
		return false;
	else
		return !strcmp(lock->classname, key->classname);
}
/*
 * Type casting
 */
ehretval_t eh_cast(const type_enum type, const ehretval_t in) {
	ehretval_t ret;
	switch(type) {
// macro for the common case
#define EH_CAST_CASE(vtype) case vtype ## _e: \
	ret = eh_xto ## vtype (in); \
	break;
		EH_CAST_CASE(int)
		EH_CAST_CASE(string)
		EH_CAST_CASE(bool)
		EH_CAST_CASE(float)
		EH_CAST_CASE(range)
		EH_CAST_CASE(array)
#undef EH_CAST_CASE
		default:
			ret.type = null_e;
			eh_error_type("typecast", type, eerror_e);
			break;
	}
	return ret;
}

#define CASTERROR(totype) do { \
	eh_error_type("typecast to " #totype, in.type, enotice_e); \
	ret.type = null_e; \
	return ret; \
} while(0)
#define CASTERROR_KNOWN(totype, vtype) do { \
	eh_error_type("typecast to " #totype, vtype ## _e, enotice_e); \
	ret.type = null_e; \
	return ret; \
} while(0)

/* Casts between specific pairs of types */
ehretval_t eh_stringtoint(const char *const in) {
	char *endptr;
	ehretval_t ret;
	ret.type = int_e;
	ret.intval = strtol(in, &endptr, 0);
	// If in == endptr, strtol read no digits and there was no conversion.
	if(in == endptr) {
		CASTERROR_KNOWN(int, string);
	}
	return ret;
}
ehretval_t eh_stringtofloat(const char *const in) {
	char *endptr;
	ehretval_t ret;
	ret.type = float_e;
	ret.floatval = strtof(in, &endptr);
	// If in == endptr, strtof read no digits and there was no conversion.
	if(in == endptr) {
		CASTERROR_KNOWN(float, string);
	}
	return ret;
}
char *eh_inttostring(const int in) {
	// INT_MAX has 10 decimal digits on this computer, so 12 (including sign and 
	// null terminator) should suffice for the result string
	char *buffer = (char *) Malloc(12);
	snprintf(buffer, 12, "%d", in);

	return buffer;
}
char *eh_floattostring(const float in) {
	char *buffer = (char *) Malloc(12);
	snprintf(buffer, 12, "%f", in);

	return buffer;
}
ehretval_t eh_rangetoarray(const ehrange_t *const range) {
	ehretval_t ret, index, member;
	ret.type = array_e;
	index.type = int_e;
	member.type = int_e;

	ret.arrayval = (ehvar_t **) Calloc(VARTABLE_S, sizeof(ehvar_t *));
	index.intval = 0;
	member.intval = range->min;
	array_insert_retval(ret.arrayval, index, member);
	index.intval = 1;
	member.intval = range->max;
	array_insert_retval(ret.arrayval, index, member);

	return ret;
}
ehretval_t eh_stringtorange(const char *const in) {
	// attempt to find two integers in the string
	ehretval_t ret;
	int min, max;
	char *ptr;
	// get lower part of range
	for(int i = 0; ; i++) {
		if(in[i] == '\0') {
			CASTERROR_KNOWN(range, string);
		}
		if(isdigit(in[i])) {
			min = strtol(&in[i], &ptr, 0);
			break;
		}
	}
	// get upper bound
	for(int i = 0; ; i++) {
		if(ptr[i] == '\0') {
			CASTERROR_KNOWN(range, string);
		}
		if(isdigit(ptr[i])) {
			max = strtol(&ptr[i], NULL, 0);
			break;
		}
	}
	return eh_make_range(min, max);
}
/* Casts from arbitrary types */
ehretval_t eh_xtoint(const ehretval_t in) {
	ehretval_t ret;
	ret.type = int_e;
	switch(in.type) {
		case int_e:
			ret.intval = in.intval;
			break;
		case string_e:
			ret = eh_stringtoint(in.stringval);
			break;
		case bool_e:
			if(in.boolval) {
				ret.intval = 1;
			} else {
				ret.intval = 0;
			}
			break;
		case null_e:
			ret.intval = 0;
			break;
		case float_e:
			ret.intval = (int) in.floatval;
			break;
		default:
			CASTERROR(int);
			break;
	}
	return ret;
}
ehretval_t eh_xtostring(const ehretval_t in) {
	ehretval_t ret;
	ret.type = string_e;
	switch(in.type) {
		case string_e:
			ret.stringval = in.stringval;
			break;
		case int_e:
			ret.stringval = eh_inttostring(in.intval);
			break;
		case null_e:
			// empty string
			ret.stringval = (char *) Malloc(1);
			ret.stringval[0] = '\0';
			break;
		case bool_e:
			if(in.boolval) {
				ret.stringval = (char *) Malloc(5);
				strcpy(ret.stringval, "true");
			}
			else {
				ret.stringval = (char *) Malloc(6);
				strcpy(ret.stringval, "false");
			}
			break;
		case float_e:
			ret.stringval = eh_floattostring(in.floatval);
			break;
		default:
			CASTERROR(string);
			break;
	}
	return ret;
}
ehretval_t eh_xtobool(const ehretval_t in) {
	ehretval_t ret;
	ret.type = bool_e;
	// convert an arbitrary variable to a bool
	switch(in.type) {
		case bool_e:
			ret.boolval = in.boolval;
			break;
		case int_e:
			ret.boolval = (in.intval != 0);
			break;
		case string_e:
			ret.boolval = (strlen(in.stringval) != 0);
			break;
		case array_e:
			// empty arrays should return false
			ret.boolval = (array_count(in.arrayval) != 0);
			break;
		case range_e:
			// 0..0 is false, everything else true
			ret.boolval = (in.rangeval->min != 0 || in.rangeval->max != 0);
			break;
		default:
			// other types are always false
			ret.boolval = false;
			break;
	}
	return ret;
}
ehretval_t eh_xtofloat(const ehretval_t in) {
	ehretval_t ret;
	ret.type = float_e;
	switch(in.type) {
		case float_e:
			ret.floatval = in.floatval;
			break;
		case int_e:
			ret.floatval = (float) in.intval;
			break;
		case string_e:
			ret = eh_stringtofloat(in.stringval);
			break;
		case bool_e:
			if(in.boolval) {
				ret.floatval = 1.0;
			} else {
				ret.floatval = 0.0;
			}
			break;
		case null_e:
			ret.floatval = 0.0;
			break;
		default:
			CASTERROR(float);
			break;
	}
	return ret;
}
ehretval_t eh_xtorange(const ehretval_t in) {
	ehretval_t ret;
	ret.type = range_e;
	switch(in.type) {
		case range_e:
			ret.rangeval = in.rangeval;
			break;
		case string_e:
			ret = eh_stringtorange(in.stringval);
			break;
		case int_e:
			ret = eh_make_range(in.intval, in.intval);
			break;
		default:
			CASTERROR(range);
			break;
	}
	return ret;
}
ehretval_t eh_xtoarray(const ehretval_t in) {
	ehretval_t ret;
	ret.type = array_e;
	switch(in.type) {
		case array_e:
			ret.arrayval = in.arrayval;
			break;
		case range_e:
			ret = eh_rangetoarray(in.rangeval);
			break;
		case int_e:
		case bool_e:
		case string_e:
		case func_e:
			// create an array with just this variable in it
			ret.arrayval = (ehvar_t **) Calloc(VARTABLE_S, sizeof(ehvar_t *));
			ehretval_t index;
			index.type = int_e;
			index.intval = 0;
			array_insert_retval(ret.arrayval, index, in);
			break;
		default:
			CASTERROR(array);
			break;
	}
	return ret;
}
static inline bool eh_floatequals(float infloat, ehretval_t operand2) {
	// checks whether a float equals an int. C handles this correctly.
	if(operand2.type != int_e) {
		operand2 = eh_xtoint(operand2);
		if(operand2.type == null_e) {
			return false;
		}
	}
	return (infloat == operand2.intval);
}
ehretval_t eh_looseequals(ehretval_t operand1, ehretval_t operand2) {
	ehretval_t ret;
	ret.type = bool_e;

	if(operand1.type == int_e && operand2.type == int_e) {
		ret.boolval = (operand1.intval == operand2.intval);
	} else if(operand1.type == string_e && operand2.type == string_e) {
		ret.boolval = !strcmp(operand1.stringval, operand2.stringval);
	} else if(operand1.type == float_e && operand2.type == float_e) {
		ret.boolval = (operand1.floatval == operand2.floatval);
	} else if(operand1.type == range_e && operand2.type == range_e) {
		ret.boolval = (operand1.rangeval->min == operand2.rangeval->min)
			&& (operand1.rangeval->max == operand2.rangeval->max);
	} else if(operand1.type == float_e) {
		ret.boolval = eh_floatequals(operand1.floatval, operand2);
	} else if(operand2.type == float_e) {
		ret.boolval = eh_floatequals(operand2.floatval, operand1);
	} else {
		operand1 = eh_xtoint(operand1);
		operand2 = eh_xtoint(operand2);
		if(operand1.type == int_e && operand2.type == int_e) {
			ret.boolval = (operand1.intval == operand2.intval);
		} else {
			ret.type = null_e;
		}
	}
	return ret;
}
ehretval_t eh_strictequals(ehretval_t operand1, ehretval_t operand2) {
	ehretval_t ret;
	ret.type = bool_e;

	if(operand1.type == int_e && operand2.type == int_e) {
		ret.boolval = (operand1.intval == operand2.intval);
	} else if(operand1.type == string_e && operand2.type == string_e) {
		ret.boolval = !strcmp(operand1.stringval, operand2.stringval);
	} else if(operand1.type == bool_e && operand2.type == bool_e) {
		ret.boolval = (operand1.boolval == operand2.boolval);
	} else if(operand1.type == null_e && operand2.type == null_e) {
		// null always equals null
		ret.boolval = true;
	} else if(operand1.type == float_e && operand2.type == float_e) {
		ret.boolval = (operand1.floatval == operand2.floatval);
	} else if(operand1.type == range_e && operand2.type == range_e) {
		ret.boolval = (operand1.rangeval->min == operand2.rangeval->min)
			&& (operand1.rangeval->max == operand2.rangeval->max);
	} else {
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
	ehvar_t *member = (ehvar_t *) Malloc(sizeof(ehvar_t));

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
		var = eh_execute(in->opval->paras[0], context);
		member->indextype = int_e;
		member->index = place;
	} else {
		label = eh_execute(in->opval->paras[0], context);
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
		var = eh_execute(in->opval->paras[1], context);
	}

	// create array member
	member->value = var;
	// set next to NULL by default
	member->next = NULL;

	// insert it into the hashtable
	ehvar_t **currptr = &array[vhash];
	switch(member->indextype) {
		case int_e:
			while(*currptr != NULL) {
				if((*currptr)->indextype == int_e 
				  && (*currptr)->index == member->index) {
					// replace this array member
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
				if((*currptr)->indextype == string_e 
				  && !strcmp((*currptr)->name, member->name)) {
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
	// Inserts a member into an array. 
	// Assumes that the member is not yet present in the array.
	unsigned int vhash;

	ehvar_t *const newvar = (ehvar_t *) Malloc(sizeof(ehvar_t));
	newvar->indextype = index.type;
	switch(index.type) {
		case int_e:
			vhash = index.intval % VARTABLE_S;
			newvar->index = index.intval;
			break;
		case string_e:
			vhash = hash(index.stringval, 0);
			newvar->name = index.stringval;
			break;
		default:
			eh_error_type("array index", index.type, enotice_e);
			free(newvar);
			return NULL;
	}
	newvar->next = array[vhash];
	array[vhash] = newvar;
	newvar->value = ret;
	return newvar;
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
			for( ; curr != NULL; curr = curr->next) {
				if(curr->indextype == int_e && curr->index == index.intval) {
					return curr;
				}
			}
			break;
		case string_e:
			for( ; curr != NULL; curr = curr->next) {
				if(curr->indextype == string_e 
				  && !strcmp(curr->name, index.stringval)) {
					return curr;
				}
			}
			break;
		default: // to keep compiler happy (issues caught by previous switch)
			break;
	}
	return NULL;
}
ehretval_t array_get(ehvar_t **array, ehretval_t index) {
	ehretval_t ret;

	const ehvar_t *curr = array_getmember(array, index);
	if(curr == NULL) {
		ret.type = null_e;
	} else {
		ret = curr->value;
	}
	return ret;
}
int array_count(ehvar_t **array) {
	// count the members of an array
	int count = 0;

	for(int i = 0; i < VARTABLE_S; i++) {
		for(ehvar_t *curr = array[i]; curr != NULL; curr = curr->next) {
			count++;
		}
	}
	return count;
}
/*
 * Variants of array access
 */
static ehretval_t int_arrow_get(ehretval_t operand1, ehretval_t operand2) {
	ehretval_t ret;

	ret.type = null_e;
	// "array" access to integer returns the nth bit of the integer; for example 
	// (assuming sizeof(int) == 32), (2 -> 30) == 1, (2 -> 31) == 0
	if(operand2.type != int_e) {
		eh_error_type("bitwise access to integer", operand2.type, enotice_e);
		return ret;
	}
	if(operand2.intval >= (int) sizeof(int) * 8) {
		eh_error_int("Identifier too large for bitwise access to integer", 	
			operand2.intval, enotice_e);
		return ret;
	}
	// get mask
	int mask = 1 << (sizeof(int) * 8 - 1);
	mask >>= operand2.intval;
	// apply mask
	ret.intval = (operand1.intval & mask) >> (sizeof(int) * 8 - 1 - mask);
	ret.type = int_e;
	return ret;
}
static ehretval_t string_arrow_get(ehretval_t operand1, ehretval_t operand2) {
	ehretval_t ret;

	ret.type = null_e;

	// "array" access to string returns integer representing nth character.
	// In the future, perhaps replace this with a char datatype or with a 
	// "shortstring" datatype representing strings up to 3 or even 4 characters 
	// long
	if(operand2.type != int_e) {
		eh_error_type("character access to string", operand2.type, enotice_e);
		return ret;
	}
	int count = strlen(operand1.stringval);
	if(operand2.intval >= count) {
		eh_error_int("Identifier too large for character access to string", 
			operand2.intval, enotice_e);
		return ret;
	}
	// get the nth character
	ret.intval = operand1.stringval[operand2.intval];
	ret.type = int_e;
	return ret;
}
static ehretval_t range_arrow_get(ehretval_t range, ehretval_t accessor) {
	ehretval_t ret;
	ret.type = null_e;
	if(accessor.type != int_e) {
		eh_error_type("arrow access to range", accessor.type, enotice_e);
		return ret;
	}
	switch(accessor.intval) {
		case 0:
			ret.type = int_e;
			ret.intval = range.rangeval->min;
			break;
		case 1:
			ret.type = int_e;
			ret.intval = range.rangeval->max;
			break;
		default:
			eh_error_int("invalid range accessor", accessor.intval, enotice_e);
			break;
	}
	return ret;
}
static void int_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue) {
	if(index.type != int_e) {
		eh_error_type("bitwise access to integer", index.type, enotice_e);
		return;
	}
	if(index.intval < 0 || (unsigned) index.intval >= sizeof(int) * 8) {
		eh_error_int("Identifier too large for bitwise access to integer", 
			index.intval, enotice_e);
		return;
	}
	// get mask
	int mask = (1 << (sizeof(int) * 8 - 1)) >> index.intval;
	if(eh_xtobool(rvalue).boolval) {
		input.referenceval->intval |= mask;
	} else {
		mask = ~mask;
		input.referenceval->intval &= mask;
	}
	return;
}
static void string_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue) {
	if(rvalue.type != int_e) {
		eh_error_type("character access to string", rvalue.type, enotice_e);
		return;
	}
	if(index.type != int_e) {
		eh_error_type("setting a character in a string", index.type, enotice_e);
		return;
	}
	int count = strlen(input.referenceval->stringval);
	if(index.intval >= count) {
		eh_error_int("Identifier too large for character access to string", 
			index.intval, enotice_e);
		return;
	}
	// get the nth character
	input.referenceval->stringval[index.intval] = rvalue.intval;
	return;
}
static void range_arrow_set(ehretval_t input, ehretval_t index, ehretval_t rvalue) {
	if(rvalue.type != int_e) {
		eh_error_type("arrow access to range", rvalue.type, enotice_e);
	} else if(index.type != int_e) {
		eh_error_type("arrow access to range", index.type, enotice_e);
	} else switch(index.intval) {
		case 0:
			input.referenceval->rangeval->min = rvalue.intval;
			break;
		case 1:
			input.referenceval->rangeval->max = rvalue.intval;
			break;
		default:
			eh_error_int("invalid range accessor", index.intval, enotice_e);
			break;
	}
	return;
}
/*
 * Other types
 */
ehretval_t eh_make_range(const int min, const int max) {
	ehretval_t ret;
	ret.type = range_e;
	ret.rangeval = (ehrange_t *) Malloc(sizeof(ehrange_t));
	ret.rangeval->min = min;
	ret.rangeval->max = max;
	return ret;
}
/*
 * Command line arguments
 */
void eh_setarg(int argc, char **argv) {
	ehretval_t ret, index;

	// insert argc
	ehvar_t *argc_v = (ehvar_t *) Malloc(sizeof(ehvar_t));
	argc_v->value.type = int_e;
	// global scope
	argc_v->scope = 0;
	argc_v->name = "argc";
	// argc - 1, because argv[0] is ehi itself
	argc_v->value.intval = argc - 1;
	insert_variable(argc_v);

	// insert argv
	ehvar_t *argv_v = (ehvar_t *) Malloc(sizeof(ehvar_t));
	argv_v->value.type = array_e;
	argv_v->scope = 0;
	argv_v->name = "argv";
	argv_v->value.arrayval = (ehvar_t **) Calloc(VARTABLE_S, sizeof(ehvar_t *));

	// all members of argv are strings
	ret.type = string_e;
	index.type = int_e;
	for(int i = 1; i < argc; i++) {
		index.intval = i - 1;
		ret.stringval = argv[i];
		array_insert_retval(argv_v->value.arrayval, index, ret);
	}
	insert_variable(argv_v);
}
