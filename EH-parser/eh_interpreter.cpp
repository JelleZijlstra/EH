/*
 * eh_interpreter.c
 * Jelle Zijlstra, December 2011
 *
 * Implements a Lex- and Yacc-based interpreter for the EH scripting language.
 */
#include "eh.h"
#include "eh_libfuncs.h"
#include "eh_libclasses.h"
#include "eh_libcmds.h"
#include <cctype>

// number of loops we're currently in
bool returning = false;
static int inloop = 0;
static int breaking = 0;
static int continuing = 0;
ehvar_t *vartable[VARTABLE_S];
ehclass_t *classtable[VARTABLE_S];
ehcmd_bucket_t *cmdtable[VARTABLE_S];

// current object, gets passed around
static ehcontext_t newcontext = NULL;
ehscope_t *global_scope = NULL;
ehscope_t *curr_scope = global_scope;

static void make_arglist(int *argcount, eharg_t **arglist, ehretval_t *node);
static ehretval_t *int_arrow_get(ehretval_t *operand1, ehretval_t *operand2);
static ehretval_t *string_arrow_get(ehretval_t *operand1, ehretval_t *operand2);
static ehretval_t *range_arrow_get(ehretval_t *operand1, ehretval_t *operand2);
static void int_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue);
static void string_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue);
static void range_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue);
// helper functions
ehretval_t *eh_count(const ehretval_t *in);
ehretval_t *eh_op_tilde(ehretval_t *in);
ehretval_t *eh_op_uminus(ehretval_t *in);
ehretval_t *eh_op_dot(ehretval_t *operand1, ehretval_t *operand2);
ehretval_t *eh_op_command(const char *name, ehretval_t *node, ehcontext_t context);
ehretval_t *eh_op_for(opnode_t *op, ehcontext_t context);
ehretval_t *eh_op_while(ehretval_t **paras, ehcontext_t context);
ehretval_t *eh_op_as(opnode_t *op, ehcontext_t context);
ehretval_t *eh_op_new(const char *name);
void eh_op_continue(opnode_t *op, ehcontext_t context);
void eh_op_break(opnode_t *op, ehcontext_t context);
ehretval_t *eh_op_array(ehretval_t *node, ehcontext_t context);
ehretval_t *eh_op_anonclass(ehretval_t *node, ehcontext_t context);
void eh_op_declarefunc(ehretval_t **paras);
ehretval_t *eh_op_declareclosure(ehretval_t **paras);
void eh_op_declareclass(ehretval_t **paras, ehcontext_t context);
ehretval_t *eh_op_switch(ehretval_t **paras, ehcontext_t context);
ehretval_t *eh_op_given(ehretval_t **paras, ehcontext_t context);
ehretval_t *eh_op_colon(ehretval_t **paras, ehcontext_t context);
ehretval_t *eh_op_reference(opnode_t *op, ehcontext_t context);
ehretval_t **eh_op_lvalue(opnode_t *op, ehcontext_t context);
ehretval_t *eh_op_dollar(ehretval_t *node, ehcontext_t context);
void eh_op_set(ehretval_t **paras, ehcontext_t context);
ehretval_t *eh_op_accessor(ehretval_t **paras, ehcontext_t context);
ehcmd_t *get_command(const char *name);
void insert_command(const ehcmd_t cmd);
void redirect_command(const char *redirect, const char *target);

ehretval_t *eh_make_range(const int min, const int max);
static inline int count_nodes(ehretval_t *node);

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

#define LIBCLASSENTRY(c) { #c, {ehlc_new_ ## c, ehlc_l_ ## c }},
ehlc_listentry_t libclasses[] = {
	LIBCLASSENTRY(CountClass)
	LIBCLASSENTRY(File)
	{NULL, {NULL, NULL}}
};

#define LIBCMDENTRY(c) { #c, ehlcmd_ ## c },
ehcmd_t libcmds[] = {
	LIBCMDENTRY(quit)
	LIBCMDENTRY(echo)
	LIBCMDENTRY(put)
	{NULL, NULL}
};

#define LIBCMDREDENTRY(r, t) { #r, #t },
const char *libredirs[][2] = {
	LIBCMDREDENTRY(q, quit)
	{NULL, NULL}
};

// macros to avoid having to check for NULL all the time
#define EH_TYPE(ret) (((ret) == NULL) ? null_e : (ret)->type)
#define DEC_RC(ret) (((ret) == NULL) ? (void)0 : (ret)->dec_rc())

/*
 * macros for interpreter behavior
 */
// take ints, return an int
#define EH_INT_CASE(token, operator) case token: \
	operand1 = eh_xtoint(eh_execute(node->opval->paras[0], context)); \
	operand2 = eh_xtoint(eh_execute(node->opval->paras[1], context)); \
	if(EH_TYPE(operand1) == int_e && EH_TYPE(operand2) == int_e) { \
		ret = new ehretval_t(int_e); \
		ret->intval = (operand1->intval operator operand2->intval); \
	} else {\
		eh_error_types(#operator, EH_TYPE(operand1), EH_TYPE(operand2), eerror_e); \
	} \
	break;
// take ints or floats, return an int or float
#define EH_FLOATINT_CASE(token, operator) case token: \
	operand1 = eh_execute(node->opval->paras[0], context); \
	operand2 = eh_execute(node->opval->paras[1], context); \
	if(EH_TYPE(operand1) == float_e && EH_TYPE(operand2) == float_e) { \
		ret = new ehretval_t(float_e); \
		ret->floatval = (operand1->floatval operator operand2->floatval); \
	} else { \
		operand1 = eh_xtoint(operand1); \
		operand2 = eh_xtoint(operand2); \
		if(EH_TYPE(operand1) == int_e && EH_TYPE(operand2) == int_e) { \
			ret = new ehretval_t(int_e); \
			ret->intval = (operand1->intval operator operand2->intval); \
		} else { \
			eh_error_types(#operator, EH_TYPE(operand1), EH_TYPE(operand1), eerror_e); \
		} \
	} \
	break;
// take ints or floats, return a bool
#define EH_INTBOOL_CASE(token, operator) case token: \
	operand1 = eh_execute(node->opval->paras[0], context); \
	operand2 = eh_execute(node->opval->paras[1], context); \
	if(EH_TYPE(operand1) == float_e && EH_TYPE(operand2) == float_e) { \
		ret = new ehretval_t(bool_e); \
		ret->boolval = (operand1->floatval operator operand2->floatval); \
	} else { \
		operand1 = eh_xtoint(operand1); \
		operand2 = eh_xtoint(operand2); \
		if(EH_TYPE(operand1) == int_e && EH_TYPE(operand2) == int_e) { \
			ret = new ehretval_t(bool_e); \
			ret->boolval = (operand1->intval operator operand2->intval); \
		} else { \
			eh_error_types(#operator, EH_TYPE(operand1), EH_TYPE(operand2), eerror_e); \
		} \
	} \
	break;
// take bools, return a bool
#define EH_BOOL_CASE(token, operator) case token: \
	operand1 = eh_xtobool(eh_execute(node->opval->paras[0], context)); \
	operand2 = eh_xtobool(eh_execute(node->opval->paras[1], context)); \
	ret = new ehretval_t(bool_e); \
	ret->boolval = (operand1->boolval operator operand2->boolval); \
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
	for(int i = 0; libfuncs[i].code != NULL; i++) {
		ehvar_t *func = new ehvar_t;
		func->name = libfuncs[i].name;
		func->scope = global_scope;
		func->value = new ehretval_t;
		func->value->type = func_e;
		func->value->funcval = new ehfm_t;
		func->value->funcval->type = lib_e;
		func->value->funcval->ptr = libfuncs[i].code;
		// other fields are irrelevant
		insert_variable(func);
	}
	for(int i = 0; libclasses[i].name != NULL; i++) {
		ehclass_t *newclass = new ehclass_t;
		newclass->type = lib_e;
		newclass->obj.classname = libclasses[i].name;
		newclass->obj.libinfo = libclasses[i].info;
		insert_class(newclass);
	}
	for(int i = 0; libcmds[i].name != NULL; i++) {
		insert_command(libcmds[i]);
	}
	for(int i = 0; libredirs[i][0] != NULL; i++) {
		redirect_command(libredirs[i][0], libredirs[i][1]);
	}
	return;
}
void eh_exit(void) {
	return;
}

/*
 * Main execution function
 */
ehretval_t *eh_execute(ehretval_t *node, const ehcontext_t context) {
	// variables used
	ehretval_t *ret, *operand1, *operand2;
	bool b1, b2;
	// default
	ret = NULL;

	// empty statements produce a null node
	if(node == NULL) {
		return ret;
	}
	if(node->type == op_e) {
		switch(node->opval->op) {
		/*
		 * Unary operators
		 */
			case '@': // type casting
				ret = eh_cast(
					node->opval->paras[0]->typeval,
					eh_execute(node->opval->paras[1], context)
				);
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
				operand1 = eh_execute(node->opval->paras[0], context);
				ret = new ehretval_t(!eh_xtobool(operand1));
				operand1->dec_rc();
				break;
		/*
		 * Control flow
		 */
			case T_IF:
				operand1 = eh_execute(node->opval->paras[0], context);
				if(eh_xtobool(operand1)) {
					ret = eh_execute(node->opval->paras[1], context);
				} else if(node->opval->nparas == 3) {
					ret = eh_execute(node->opval->paras[2], context);
				}
				if(operand1 != NULL) {
					operand1->dec_rc();
				}
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
				if(node->opval->nparas == 0) {
					return ret;
				}
				// else execute both commands
				ret = eh_execute(node->opval->paras[0], context);
				if(returning || breaking || continuing) {
					return ret;
				}
				ret = eh_execute(node->opval->paras[1], context);
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
				ret = eh_op_new(
					eh_execute(node->opval->paras[0], context)->stringval
				);
				break;
		/*
		 * Object definitions
		 */
			case T_FUNC: // function definition
				ret = eh_op_declareclosure(node->opval->paras);
				break;
			case T_CLASS: // class declaration
				eh_op_declareclass(node->opval->paras, context);
				break;
			case T_ATTRIBUTE: // class member attributes
				if(node->opval->nparas == 0) {
					ret = new ehretval_t(attributestr_e);
					// all zeroes
					ret->intval = 0;
				} else {
					// first execute first para
					ret = eh_execute(node->opval->paras[0], context);
					// then overwrite with attribute from second para
					switch(node->opval->paras[1]->attributeval) {
						case publica_e:
							ret->attributestrval.visibility = public_e;
							break;
						case privatea_e:
							ret->attributestrval.visibility = private_e;
							break;
						case statica_e:
							ret->attributestrval.isstatic = static_e;
							break;
						case consta_e:
							ret->attributestrval.isconst = const_e;
							break;
					}
				}
				break;
			case '[': // array declaration
				ret = eh_op_array(node->opval->paras[0], context);
				break;
			case '{': // anonymous class
				ret = eh_op_anonclass(node->opval->paras[0], context);
				break;
		/*
		 * Binary operators
		 */
			case '=': // equality
				ret = eh_looseequals(
					eh_execute(node->opval->paras[0], context),
					eh_execute(node->opval->paras[1], context)
				);
				break;
			case T_SE: // strict equality
				ret = new ehretval_t(eh_strictequals(
					eh_execute(node->opval->paras[0], context),
					eh_execute(node->opval->paras[1], context)
				));
				break;
			case T_SNE: // strict non-equality
				ret = new ehretval_t(!eh_strictequals(
					eh_execute(node->opval->paras[0], context),
					eh_execute(node->opval->paras[1], context)
				));
				break;
			EH_INTBOOL_CASE('>', >) // greater-than
			EH_INTBOOL_CASE('<', <) // lesser-than
			EH_INTBOOL_CASE(T_GE, >=) // greater-than or equal
			EH_INTBOOL_CASE(T_LE, <=) // lesser-than or equal
			EH_INTBOOL_CASE(T_NE, !=) // not equal
			EH_FLOATINT_CASE('+', +) // addition
			case '.': // string concatenation
				ret = eh_op_dot(
					eh_execute(node->opval->paras[0], context),
					eh_execute(node->opval->paras[1], context)
				);
				break;
			EH_FLOATINT_CASE('-', -) // subtraction
			EH_FLOATINT_CASE('*', *) // multiplication
			EH_FLOATINT_CASE('/', /) // division
			EH_INT_CASE('%', %) // modulo
			EH_INT_CASE('&', &) // bitwise AND
			EH_INT_CASE('^', ^) // bitwise XOR
			EH_INT_CASE('|', |) // bitwise OR
			case T_AND: // AND; use short-circuit operation
				operand1 = eh_execute(node->opval->paras[0], context);
				if(!eh_xtobool(operand1)) {
					ret = new ehretval_t(false);
				} else {
					operand2 = eh_execute(node->opval->paras[1], context);
					ret = new ehretval_t(eh_xtobool(operand2));
					DEC_RC(operand2);
				}
				DEC_RC(operand1);
				break;
			case T_OR: // OR; use short-circuit operation
				operand1 = eh_execute(node->opval->paras[0], context);
				if(eh_xtobool(operand1)) {
					ret = new ehretval_t(true);
				} else {
					operand2 = eh_execute(node->opval->paras[1], context);
					ret = new ehretval_t(eh_xtobool(operand2));
					DEC_RC(operand2);
				}
				DEC_RC(operand1);
				break;
			case T_XOR:
				operand1 = eh_execute(node->opval->paras[0], context);
				operand2 = eh_execute(node->opval->paras[1], context);
				b1 = eh_xtobool(operand1);
				b2 = eh_xtobool(operand2);
				operand1->dec_rc();
				operand2->dec_rc();
				ret = new ehretval_t((b1 && !b2) || (!b1 && b2));
				break;
		/*
		 * Variable manipulation
		 */
			case T_LVALUE_GET:
			case T_LVALUE_SET:
				ret = *eh_op_lvalue(node->opval, context);
				break;
			case T_RANGE:
				// Attempt to cast operands to integers; if this does not work,
				// return NULL. No need to yell, since eh_xtoi already does
				// that.
				operand1 = eh_xtoint(eh_execute(node->opval->paras[0], context));
				if(operand1->type == null_e)
					break;
				operand2 = eh_xtoint(eh_execute(node->opval->paras[1], context));
				if(operand2->type == null_e)
					break;
				ret = eh_make_range(operand1->intval, operand2->intval);
				break;
			case T_SET:
				eh_op_set(node->opval->paras, context);
				break;
			case T_MINMIN:
				operand1 = eh_execute(node->opval->paras[0], context);
				if(operand1 == NULL) {
					eh_error("Cannot set with -- operator", eerror_e);
				} else {
					switch(operand1->type) {
						case int_e:
							operand1->intval--;
							break;
						default:
							eh_error_type("-- operator", operand1->type, eerror_e);
							break;
					}
					operand1->dec_rc();
				}
				break;
			case T_PLUSPLUS:
				operand1 = eh_execute(node->opval->paras[0], context);
				if(operand1 == NULL) {
					eh_error("Cannot set with ++ operator", eerror_e);
				} else {
					switch(operand1->type) {
						case int_e:
							operand1->intval++;
							break;
						default:
							eh_error_type("++ operator", operand1->type, eerror_e);
							break;
					}
					operand1->dec_rc();
				}
				break;
			case T_REFERENCE: // reference declaration
				ret = eh_op_reference(node->opval->paras[0]->opval, context);
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
					eh_execute(node->opval->paras[0], context)->stringval,
					node->opval->paras[1],
					context
				);
				break;
			default:
				eh_error_int("Unexpected opcode", node->opval->op, efatal_e);
				break;
		}
	} else {
		ret = node;
	}
	return ret;
}
/*
 * Opnode execution helpers
 */
void print_retval(const ehretval_t *ret) {
	if(ret == NULL) {
		printf("(null)");
	} else switch(ret->type) {
		case string_e:
			printf("%s", ret->stringval);
			break;
		case int_e:
			printf("%d", ret->intval);
			break;
		case bool_e:
			if(ret->boolval) {
				printf("(true)");
			} else {
				printf("(false)");
			}
			break;
		case null_e:
			printf("(null)");
			break;
		case float_e:
			printf("%f", ret->floatval);
			break;
		case range_e:
			printf("%d to %d", ret->rangeval->min, ret->rangeval->max);
			break;
		default:
			eh_error_type("echo operator", ret->type, enotice_e);
			break;
	}
	return;
}
ehretval_t *eh_count(const ehretval_t *in) {
	ehretval_t *ret = new ehretval_t;
	ret->type = int_e;
	switch(in->type) {
		case int_e:
			ret->intval = sizeof(int) * 8;
			break;
		case float_e:
			ret->intval = sizeof(float) * 8;
			break;
		case string_e:
			ret->intval = strlen(in->stringval);
			break;
		case array_e:
			ret->intval = array_count(in->arrayval);
			break;
		case null_e:
			ret->intval = 0;
			break;
		case bool_e:
			ret->intval = 0;
			break;
		case range_e:
			ret->intval = 2;
			break;
		default:
			eh_error_type("count operator", in->type, eerror_e);
			ret->type = null_e;
			break;
	}
	return ret;
}
ehretval_t *eh_op_tilde(ehretval_t *in) {
	// no const argument because it's modified below
	ehretval_t *ret = NULL;
	switch(in->type) {
		// bitwise negation of a bool is just normal negation
		case bool_e:
			ret = new ehretval_t(!in->boolval);
			break;
		// else try to cast to int
		default:
			in = eh_xtoint(in);
			if(in->type != int_e) {
				eh_error_type("bitwise negation", in->type, eerror_e);
				return ret;
			}
			// fall through to int case
		case int_e:
			ret = new ehretval_t(~in->intval);
			break;
	}
	return ret;
}
ehretval_t *eh_op_uminus(ehretval_t *in) {
	ehretval_t *ret = NULL;
	switch(in->type) {
		// negation
		case bool_e:
			ret = new ehretval_t(!in->boolval);
			break;
		case float_e:
			ret = new ehretval_t(-in->floatval);
			break;
		default:
			in = eh_xtoint(in);
			if(in->type != int_e) {
				eh_error_type("negation", in->type, eerror_e);
				return ret;
			}
			// fall through to int case
		case int_e:
			ret = new ehretval_t(-in->intval);
			break;
	}
	return ret;
}
ehretval_t *eh_op_dot(ehretval_t *operand1, ehretval_t *operand2) {
	ehretval_t *ret = NULL;
	operand1 = eh_xtostring(operand1);
	operand2 = eh_xtostring(operand2);
	if(operand1->type == string_e && operand2->type == string_e) {
		// concatenate them
		ret = new ehretval_t(string_e);
		size_t len1 = strlen(operand1->stringval);
		size_t len2 = strlen(operand2->stringval);
		ret->stringval = new char[len1 + len2 + 1];
		strcpy(ret->stringval, operand1->stringval);
		strcpy(ret->stringval + len1, operand2->stringval);
	}
	return ret;
}
ehretval_t *eh_op_command(const char *name, ehretval_t *node, ehcontext_t context) {
	ehretval_t *index_r;
	ehretval_t *value_r;
	ehvar_t **paras;
	ehretval_t *node2;
	// count for simple parameters
	int count = 0;
	// we're making an array of parameters
	paras = new ehvar_t *[VARTABLE_S]();
	// loop through the paras given
	for( ; node->opval->nparas != 0; node = node->opval->paras[1]) {
		node2 = node->opval->paras[0];
		if(node2->type == op_e) {
			switch(node2->opval->op) {
				case T_SHORTPARA:
					// short paras: set each short-form option to the same thing
					if(node2->opval->nparas == 2) {
						// set to something else if specified
						value_r = eh_execute(node2->opval->paras[1], context);
					} else {
						// set to true by default
						value_r = new ehretval_t(true);
					}
					node2 = node2->opval->paras[0];
					for(int i = 0, len = strlen(node2->stringval); i < len; i++) {
						index_r = new ehretval_t(string_e);
						index_r->stringval = new char[2];
						index_r->stringval[0] = node2->stringval[i];
						index_r->stringval[1] = '\0';
						array_insert_retval(paras, index_r, value_r);
					}
					break;
				case T_LONGPARA:
					// long-form paras
					if(node2->opval->nparas == 1) {
						value_r = new ehretval_t(true);
						array_insert_retval(
							paras,
							eh_execute(node2->opval->paras[0], context),
							value_r
						);
					} else {
						array_insert_retval(
							paras,
							eh_execute(node2->opval->paras[0], context),
							eh_execute(node2->opval->paras[1], context)
						);
					}
					break;
				case T_REDIRECT:
					index_r = new ehretval_t(string_e);
					index_r->stringval = new char[sizeof(">")];
					strcpy(index_r->stringval, ">");
					// output redirector
					array_insert_retval(
						paras,
						index_r,
						eh_execute(node2->opval->paras[0], context)
					);
					break;
				case '}':
					index_r = new ehretval_t(string_e);
					index_r->stringval = new char[sizeof("}")];
					strcpy(index_r->stringval, "}");
					// output redirector
					array_insert_retval(
						paras,
						index_r,
						eh_execute(node2->opval->paras[0], context)
					);
					break;
				default: // non-named parameters with an expression
					// non-named parameters
					index_r = new ehretval_t(int_e);
					index_r->intval = count;
					value_r = eh_execute(node2, context);
					array_insert_retval(paras, index_r, value_r);
					count++;
					break;
			}
		} else {
			// non-named parameters
			index_r = new ehretval_t(int_e);
			index_r->intval = count;
			value_r = eh_execute(node2, context);
			array_insert_retval(paras, index_r, value_r);
			count++;
		}
	}
	// insert indicator that this is an EH-PHP command
	value_r = new ehretval_t(true);
	index_r = new ehretval_t(strdup("_ehphp"));
	array_insert_retval(paras, index_r, value_r);
	// get the command to execute
	const ehcmd_t *libcmd = get_command(name);
	ehretval_t *ret;
	if(libcmd != NULL) {
		ret = libcmd->code(paras);
	} else {
		ret = interpreter->execute_cmd(name, paras);
	}
	// we're not returning anymore
	returning = false;
	delete paras;
	return ret;
}
ehretval_t *eh_op_for(opnode_t *op, ehcontext_t context) {
	ehretval_t *ret = NULL;
	inloop++;
	breaking = 0;
	ehrange_t range;

	// get the count
	ehretval_t *count_r = eh_execute(op->paras[0], context);
	if(count_r->type == range_e) {
		range = *count_r->rangeval;
	} else {
		count_r = eh_xtoint(count_r);
		if(count_r->type != int_e) {
			eh_error_type("count", count_r->type, eerror_e);
			return ret;
		}
		range.min = 0;
		range.max = count_r->intval - 1;
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
		ehvar_t *var = get_variable(name, curr_scope, context, T_LVALUE_SET);
		// if we do T_LVALUE_SET, get_variable never returns NULL
		// count variable always gets to be an int
		if(var->value == NULL) {
			var->value = new ehretval_t((int) range.min);
		} else {
			ehretval_t newvalue((int) range.min);
			var->value = var->value->overwrite(&newvalue);
		}
		for( ; var->value->intval <= range.max; var->value->intval++) {
			ret = eh_execute(op->paras[2], context);
			LOOPCHECKS;
		}
	}
	inloop--;
	return ret;
}
ehretval_t *eh_op_while(ehretval_t **paras, ehcontext_t context) {
	ehretval_t *ret = NULL;
	inloop++;
	breaking = 0;
	while(eh_xtobool(eh_execute(paras[0], context))) {
		ret = eh_execute(paras[1], context);
		LOOPCHECKS;
	}
	inloop--;
	return ret;
}
ehretval_t *eh_op_as(opnode_t *op, ehcontext_t context) {
	ehretval_t *ret = NULL;

	// get the object to be looped through and check its type
	ehretval_t *object = eh_execute(op->paras[0], context);
	if(object->type != array_e && object->type != object_e) {
		eh_error_type("for ... as operator", object->type, enotice_e);
		return ret;
	}
	// increment loop count
	inloop++;
	// establish variables
	char *membername;
	ehvar_t *membervar;
	char *indexname;
	ehvar_t *indexvar = NULL;
	ehretval_t *code;
	if(op->nparas == 3) {
		// no index
		membername = op->paras[1]->stringval;
		indexname = NULL;
		code = op->paras[2];
	} else {
		// with index
		indexname = op->paras[1]->stringval;
		membername = op->paras[2]->stringval;
		code = op->paras[3];
	}
	// create variables
	membervar = get_variable(membername, curr_scope, context, T_LVALUE_SET);
	if(indexname != NULL) {
		indexvar = get_variable(indexname, curr_scope, context, T_LVALUE_SET);
	}
	if(object->type == object_e) {
		// object index is always a string
		if(indexname) {
			indexvar->value->type = string_e;
		}
		ehvar_t **members = object->objectval->members;
		// check whether we're allowed to access private things
		const bool doprivate = ehcontext_compare(object->objectval, context);
		for(int i = 0; i < VARTABLE_S; i++) {
			for(ehvar_t *currmember = members[i]; currmember != NULL; currmember = currmember->next) {
				// ignore private
				if(!doprivate && currmember->attribute.visibility == private_e) {
					continue;
				}
				if(currmember->attribute.isconst == const_e) {
					// test whether this works
					membervar->value->type = creference_e;
					membervar->value->referenceval = currmember->value;
				} else {
					membervar->value = currmember->value;
				}
				if(indexname) {
					// need the strdup here because currmember->name is const
					// and a string_e is not. Perhaps solve this instead by
					// creating a new cstring_e type?
					indexvar->value->stringval = strdup(currmember->name);
				}
				ret = eh_execute(code, context);
				LOOPCHECKS;
			}
		}
	} else {
		// arrays
		ehvar_t **members = object->arrayval;
		for(int i = 0; i < VARTABLE_S; i++) {
			for(ehvar_t *currmember = members[i]; currmember != NULL; currmember = currmember->next) {
				membervar->value = currmember->value;
				if(indexname) {
					indexvar->value->type = currmember->indextype;
					if(currmember->indextype == string_e) {
						indexvar->value->stringval = strdup(currmember->name);
					} else {
						indexvar->value->intval = currmember->index;
					}
				}
				ret = eh_execute(code, context);
				LOOPCHECKS;
			}
		}
	}
	inloop--;
	return ret;
}
ehretval_t *eh_op_new(const char *name) {
	ehretval_t *ret = NULL;

	ehclass_t *classobj = get_class(name);
	if(classobj == NULL) {
		eh_error_unknown("class", name, efatal_e);
		return ret;
	}
	ret = new ehretval_t(object_e);
	ret->objectval = new ehobj_t;
	ret->objectval->classname = name;
	ret->objectval->members = new ehvar_t *[VARTABLE_S]();
	if(classobj->type == user_e) {
		for(int i = 0; i < VARTABLE_S; i++) {
			for(ehvar_t *m = classobj->obj.members[i]; m != NULL; m = m->next) {
				class_copy_member(ret->objectval, m, i);
			}
		}
	} else {
		// insert selfptr
		ret->objectval->selfptr = classobj->obj.libinfo.constructor();
		// library classes
		ehlibentry_t *members = classobj->obj.libinfo.members;
		// attributes for library methods
		memberattribute_t attributes;
		attributes.visibility = public_e;
		attributes.isstatic = nonstatic_e;
		attributes.isconst = nonconst_e;
		// value
		ehretval_t *value;
		for(int i = 0; members[i].name != NULL; i++) {
			value = new ehretval_t(func_e);
			value->funcval = new ehfm_t;
			value->funcval->type = libmethod_e;
			value->funcval->mptr = members[i].func;
			class_insert_retval(
				ret->objectval->members, members[i].name, attributes, value
			);
		}
	}
	return ret;
}
void eh_op_break(opnode_t *op, ehcontext_t context) {
	int level;
	if(op->nparas == 0) {
		level = 1;
	} else {
		ehretval_t *level_v = eh_xtoint(eh_execute(op->paras[0], context));
		if(level_v->type != int_e) {
			level_v->dec_rc();
			return;
		} else {
			level = level_v->intval;
			level_v->dec_rc();
		}
	}
	// break as many levels as specified by the argument
	if(level > inloop) {
		eh_error_looplevels("Cannot break", level);
		return;
	}
	breaking = level;
	return;
}
void eh_op_continue(opnode_t *op, ehcontext_t context) {
	int level;
	if(op->nparas == 0) {
		level = 1;
	} else {
		ehretval_t *level_v = eh_xtoint(eh_execute(op->paras[0], context));
		if(level_v->type != int_e) {
			level_v->dec_rc();
			return;
		} else {
			level = level_v->intval;
			level_v->dec_rc();
		}
	}
	// break as many levels as specified by the argument
	if(level > inloop) {
		eh_error_looplevels("Cannot continue", level);
		return;
	}
	continuing = level;
	return;
}
ehretval_t *eh_op_array(ehretval_t *node, ehcontext_t context) {
	ehretval_t *ret = new ehretval_t(array_e);
	ret->arrayval = new ehvar_t *[VARTABLE_S]();
	// need to count array members first, because they are reversed in our node.
	// That's not necessary with functions (where the situation is analogous), because the reversals that happen when parsing the prototype argument list and parsing the argument list in a call cancel each other out.
	int count = 0;
	for(ehretval_t *node2 = node; node2->opval->nparas != 0; node2 = node2->opval->paras[0]) {
		count++;
	}
	for(ehretval_t *node2 = node; node2->opval->nparas != 0; node2 = node2->opval->paras[0]) {
		array_insert(ret->arrayval, node2->opval->paras[1], --count, context);
	}
	return ret;
}
ehretval_t *eh_op_anonclass(ehretval_t *node, ehcontext_t context) {
	ehretval_t *ret = new ehretval_t(object_e);
	ret->objectval = new ehobj_t;
	ret->objectval->classname = "AnonClass";
	ret->objectval->members = new ehvar_t *[VARTABLE_S]();
	// all members are public, non-static, non-const
	memberattribute_t attributes;
	attributes.visibility = public_e;
	attributes.isconst = nonconst_e;
	attributes.isstatic = nonstatic_e;
	ehvar_t **members = ret->objectval->members;

	for( ; node->opval->nparas != 0; node = node->opval->paras[0]) {
		ehretval_t **myparas = node->opval->paras[1]->opval->paras;
		// nodes here will always have the name in para 0 and value in para 1
		ehretval_t *namev = eh_execute(myparas[0], context);
		if(namev->type != string_e) {
			eh_error_type("Class member label", namev->type, eerror_e);
			continue;
		}
		ehretval_t *value = eh_execute(myparas[1], context);
		class_insert_retval(members, namev->stringval, attributes, value);
	}
	return ret;
}
ehretval_t *eh_op_declareclosure(ehretval_t **paras) {
	ehretval_t *ret = new ehretval_t(func_e);
	ret->funcval = new ehfm_t;
	ret->funcval->type = user_e;
	make_arglist(&ret->funcval->argcount, &ret->funcval->args, paras[0]);
	ret->funcval->code = paras[1];
	return ret;
}
void eh_op_declareclass(ehretval_t **paras, ehcontext_t context) {
	ehretval_t *classname_r = eh_execute(paras[0], context);
	ehclass_t *classobj = get_class(classname_r->stringval);
	if(classobj != NULL) {
		eh_error_redefine("class", classname_r->stringval, efatal_e);
		return;
	}
	classobj = new ehclass_t;
	classobj->type = user_e;
	classobj->obj.classname = classname_r->stringval;
	classobj->obj.members = new ehvar_t *[VARTABLE_S]();
	// insert class members
	for(ehretval_t *node = paras[1]; node != NULL; node = node->opval->paras[1]) {
		if(node->type == op_e && node->opval->op == ',') {
			class_insert(classobj->obj.members, node->opval->paras[0], context);
		} else {
			class_insert(classobj->obj.members, node, context);
			break;
		}
	}
	// insert "this" pointer
	memberattribute_t thisattributes;
	thisattributes.visibility = private_e;
	thisattributes.isstatic = nonstatic_e;
	thisattributes.isconst = const_e;
	ehretval_t *thisvalue = new ehretval_t(object_e);
	thisvalue->objectval = &(classobj->obj);
	class_insert_retval(
		classobj->obj.members, "this", thisattributes, thisvalue
	);
	insert_class(classobj);
	return;
}
ehretval_t *eh_op_switch(ehretval_t **paras, ehcontext_t context) {
	ehretval_t *ret = NULL;
	// because we use continue, we'll pretend this is a loop
	inloop++;

	// switch variable
	ehretval_t *switchvar = eh_execute(paras[0], context);
	for(ehretval_t *node = paras[1]; node->opval->nparas != 0; node = node->opval->paras[1]) {
		opnode_t *op = node->opval->paras[0]->opval;
		// execute default
		if(op->nparas == 1) {
			ret = eh_execute(op->paras[0], context);
		} else {
			ehretval_t *casevar = eh_execute(op->paras[0], context);
			ehretval_t *decider;
			// try to call function
			if(casevar->type == func_e) {
				decider = call_function_args(casevar->funcval, context, newcontext, 1, switchvar);
				if(decider->type != bool_e) {
					eh_error("Switch case method does not return bool", eerror_e);
					return NULL;
				}
			} else {
				decider = eh_looseequals(switchvar, casevar);
			}
			// apply the decider
			if(decider->boolval) {
				ret = eh_execute(op->paras[1], context);
			} else {
				continue;
			}
		}
		// check whether we need to leave
		if(returning) {
			return ret;
		} else if(breaking) {
			breaking--;
			return ret;
		} else if(continuing) {
			// if continuing == 1, then continue
			continuing--;
			// so if continuing now > 0, leave the switch
			if(continuing) {
				return ret;
			}
		}
		else {
			return ret;
		}
	}
	return NULL;
}
ehretval_t *eh_op_given(ehretval_t **paras, ehcontext_t context) {
	// switch variable
	ehretval_t *switchvar = eh_execute(paras[0], context);
	for(ehretval_t *node = paras[1]; node->opval->nparas != 0; node = node->opval->paras[1]) {
		const opnode_t *op = node->opval->paras[0]->opval;
		// execute default
		if(op->nparas == 1) {
			return eh_execute(op->paras[0], context);
		}
		ehretval_t *casevar = eh_execute(op->paras[0], context);
		ehretval_t *decider;
		if(casevar->type == func_e) {
			decider = call_function_args(
				casevar->funcval, context, newcontext, 1, switchvar
			);
			if(decider->type != bool_e) {
				eh_error("Given case method does not return bool", eerror_e);
				return NULL;
			}
		} else {
			decider = eh_looseequals(switchvar, casevar);
		}
		if(decider->boolval) {
			return eh_execute(op->paras[1], context);
		}
	}
	return NULL;
}
ehretval_t *eh_op_colon(ehretval_t **paras, ehcontext_t context) {
	ehretval_t *ret = NULL;
	ehvar_t *func;

	newcontext = NULL;
	ehretval_t *function = eh_execute(paras[0], context);
	// if we didn't find a function
	if(function == NULL) {
		return ret;
	}
	// operand1 will be either a string (indicating a normal function call) or a 
	// func_e (indicating a method or closure call)
	switch(function->type) {
		case string_e:
			func = get_variable(
				function->stringval, curr_scope, context, T_LVALUE_GET
			);
			if(func == NULL) {
				eh_error_unknown("function", function->stringval, efatal_e);
				return ret;
			}
			if(func->value->type != func_e) {
				eh_error_type("function call", func->value->type, eerror_e);
				return ret;
			}
			ret = call_function(
				func->value->funcval, paras[1], context, context
			);
			break;
		case func_e:
			ret = call_function(
				function->funcval, paras[1], context, newcontext
			);
			break;
		default:
			eh_error_type("function call", function->type, eerror_e);
			break;
	}
	function->dec_rc();
	return ret;
}
ehretval_t *eh_op_reference(opnode_t *op, ehcontext_t context) {
	ehretval_t **var = eh_op_lvalue(op, context);
	if(var == NULL) {
		eh_error("Unable to create reference", eerror_e);
	}
	ehretval_t *ret = new ehretval_t(reference_e);
	ret->referenceval = (*var)->reference(var);
	return ret;
}
ehretval_t **eh_op_lvalue(opnode_t *op, ehcontext_t context) {
	/*
	 * Get an lvalue. This function normally returns a pointer to a pointer
	 * to an ehretval_t: it points to the place where the pointer to the value
	 * of the variable resides.
	 *
	 * Because of special needs of calling code, this function sometimes breaks
	 * the normal conventions associating ehretval_t::type values with the
	 * values in the ehretval_t::value union. If the lvalue does not exist,
	 * it returns NULL.
	 *
	 * Otherwise, it returns an attribute_e with a pointer to the ehretval_t of
	 * the variable referred to, so that eh_op_set can do its bitwise magic with
	 * ints and similar stuff.
	 */
	ehretval_t **ret = NULL;
	ehvar_t *var;

	ehretval_t *basevar = eh_execute(op->paras[0], context);
	// We need this because of code in eh_op_set checking for this. Removing
	// the check for NULL there yields a problem where $ foo->2 = 0 produces 
	// $foo = @int 0.
	switch(op->nparas) {
		case 1:
			var = get_variable(basevar->stringval, curr_scope, context, op->op);
			// dereference variable
			if(var != NULL) {
				// increase refcount?
				ret = &(var->value);
			}
			/*
			 * If there is no variable of this name, and it is a
			 * simple access, we use NULL as the return value.
			 */
			break;
		case 3:
			switch(op->paras[1]->accessorval) {
				case arrow_e:
					ret = object_access(basevar, op->paras[2], context, op->op);
					break;
				case doublecolon_e:
					ret = colon_access(basevar, op->paras[2], context, op->op);
					break;
			}
			break;
	}
	basevar->dec_rc();
	return ret;
}
ehretval_t *eh_op_dollar(ehretval_t *node, ehcontext_t context) {
	ehretval_t *ret = eh_execute(node, context);
	if(ret == NULL) {
		return ret;
	}
	
	ehretval_t *varname = eh_xtostring(ret);
	if(varname == NULL) {
		return ret;
	}
	
	ehvar_t *var = get_variable(
		varname->stringval, curr_scope, context, T_LVALUE_GET
	);
	if(var == NULL || var->value == NULL) {
		ret = new ehretval_t(null_e);
	} else {
		ret = var->value;
		// do we need this?
		while(ret->type == creference_e) {
			ret = ret->referenceval;
		}
	}
	return ret;
}
void eh_op_set(ehretval_t **paras, ehcontext_t context) {
	ehretval_t **lvalue = eh_op_lvalue(paras[0]->opval, context);
	ehretval_t *rvalue = eh_execute(paras[1], context);
	if(rvalue != NULL) {
		rvalue->inc_rc();
	}
	if(lvalue == NULL) {
		// do nothing
		return;
	} else if((*lvalue)->type == attribute_e) {
		// lvalue is a pointer to the variable modified, rvalue is the value set to, index is the index
		ehretval_t *index = eh_execute(paras[0]->opval->paras[2], context);
		switch((*lvalue)->referenceval->type) {
			case int_e:
				int_arrow_set((*lvalue)->referenceval, index, rvalue);
				break;
			case string_e:
				string_arrow_set((*lvalue)->referenceval, index, rvalue);
				break;
			case range_e:
				range_arrow_set((*lvalue)->referenceval, index, rvalue);
				break;
			default:
				eh_error_type("arrow access", (*lvalue)->referenceval->type, eerror_e);
				break;
		}
		index->dec_rc();
	} else {
		if(EH_TYPE(rvalue) == reference_e) {
			// set new reference
			*lvalue = rvalue->referenceval;
			rvalue->dec_rc();
		} else {
			// set variable
			*lvalue = (*lvalue)->overwrite(rvalue);
		}
	}
	return;
}
ehretval_t *eh_op_accessor(ehretval_t **paras, ehcontext_t context) {
	// this only gets executed for array-type int and string access
	ehretval_t *ret = NULL;
	ehretval_t *basevar = eh_execute(paras[0], context);
	ehretval_t *index = eh_execute(paras[2], context);
	switch(paras[1]->accessorval) {
		case arrow_e:
			// "array" access
			switch(basevar->type) {
				case int_e:
					ret = int_arrow_get(basevar, index);
					break;
				case string_e:
					ret = string_arrow_get(basevar, index);
					break;
				case range_e:
					ret = range_arrow_get(basevar, index);
					break;
				case array_e:
					// array access to an array works as expected.
					ret = array_get(basevar->arrayval, index);
					break;
				case object_e:
					if(index->type != string_e) {
						eh_error_type("access to object", index->type, eerror_e);
					} else {
						ret = class_get(
							basevar->objectval, index->stringval, context
						);
						if(ret != NULL) {
							newcontext = basevar->objectval;
						} else {
							eh_error_unknown(
								"object member", index->stringval, eerror_e
							);
						}
					}
					break;
				default:
					eh_error_type("arrow access", basevar->type, eerror_e);
					break;
			}
			break;
		case doublecolon_e:
			ret = *colon_access(basevar, index, context, T_LVALUE_GET);
			break;
		default:
			eh_error("Unsupported accessor", efatal_e);
			break;
	}
	basevar->dec_rc();
	index->dec_rc();
	return ret;
}

/*
 * Variables
 */
bool insert_variable(ehvar_t *var) {
	unsigned int vhash;
	//printf("Inserting variable %s with value %d at scope %d\n", var->name, var->intval, var->scope);
	vhash = hash(var->name, (uint32_t) var->scope);
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
ehvar_t *get_variable(const char *name, ehscope_t *scope, ehcontext_t context, int token) {
	ehvar_t *currvar;

	// try the object first
	if(context != NULL) {
		currvar = class_getmember(context, name, context);
		if(currvar != NULL) {
			return currvar;
		}
	}
	// look in this scope, then the parent scope
	while(1) {
		unsigned int vhash = hash(name, (uint32_t) scope);
		currvar = vartable[vhash];
		while(currvar != NULL) {
			if(strcmp(currvar->name, name) == 0 && currvar->scope == scope) {
				return currvar;
			}
			currvar = currvar->next;
		}
		if(scope == NULL) {
			break;
		} else {
			scope = scope->parent;
		}
	}
	if(token == T_LVALUE_SET) {
		currvar = new ehvar_t;
		currvar->value = new ehretval_t(null_e);
		currvar->name = name;
		currvar->scope = curr_scope;
		insert_variable(currvar);
		return currvar;
	} else {
		return NULL;
	}
}
// remove all variables in scope scope
void remove_scope(ehscope_t *scope) {
	for(int i = 0; i < VARTABLE_S; i++) {
		ehvar_t *c = vartable[i];
		ehvar_t *p = NULL;
		while(c != NULL) {
			if(c->scope == scope) {
				if(p == NULL) {
					vartable[i] = c->next;
				} else {
					p->next = c->next;
				}
				delete c;
			}
			p = c;
			c = c->next;
		}
	}
}
void remove_variable(const char *name, ehscope_t *scope) {
	const unsigned int vhash = hash(name, (uint32_t) scope);
	ehvar_t *currvar = vartable[vhash];
	ehvar_t *prevvar = NULL;
	while(currvar != NULL) {
		if(strcmp(currvar->name, name) == 0 && currvar->scope == scope) {
			if(prevvar == NULL) {
				vartable[vhash] = currvar->next;
			} else {
				prevvar->next = currvar->next;
			}
			delete currvar;
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
			printf(
				"Variable %s of type %d at scope %d in hash %d at address %x\n", 
				tmp->name, tmp->value->type, (uint32_t) tmp->scope, i, 
				(int) tmp
			);
			tmp = tmp->next;
		}
	}
}
/*
 * Functions
 */
static void make_arglist(int *argcount, eharg_t **arglist, ehretval_t *node) {
	int currarg = count_nodes(node);
	*argcount = currarg;
	// if there are no arguments, the arglist can be NULL
	if(currarg) {
		*arglist = new eharg_t;
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
ehretval_t *call_function(ehfm_t *f, ehretval_t *args, ehcontext_t context, ehcontext_t newcontext) {
	ehretval_t *ret = NULL;

	if(f->type == lib_e) {
		// library function
		f->ptr(args, &ret, context);
		return ret;
	} else if(f->type == libmethod_e) {
		if(newcontext == NULL) {
			eh_error("Bare call of library method", eerror_e);
			return NULL;
		}
		f->mptr(newcontext->selfptr, args, &ret, newcontext);
		return ret;
	}
	int i = 0;
	// create new scope
	ehscope_t *new_scope = new ehscope_t;
	new_scope->parent = curr_scope;
	
	// set parameters as necessary
	if(f->args == NULL) {
		if(args->opval->nparas != 0) {
			eh_error_argcount(f->argcount, 1);
			return ret;
		}
	}
	else while(args->opval->nparas != 0) {
		ehvar_t *var = new ehvar_t;
		var->name = f->args[i].name;
		var->scope = new_scope;
		insert_variable(var);
		i++;
		if(i > f->argcount) {
			eh_error_argcount(f->argcount, i);
			return ret;
		}
		var->value = eh_execute(args->opval->paras[1], context);
		// if it's a reference, dereference it
		if(var->value->type == reference_e) {
			ehretval_t *tmp = var->value;
			var->value = var->value->referenceval;
			delete tmp;
		} else {
			var->value = var->value->share();
		}
		args = args->opval->paras[0];
	}
	// functions get their own scope (not incremented before because execution of arguments needs parent scope)
	curr_scope = new_scope;
	if(f->argcount != i) {
		eh_error_argcount(f->argcount, i);
		return ret;
	}
	// set new context (only useful for methods)
	ret = eh_execute(f->code, newcontext);
	returning = false;
	
	remove_scope(curr_scope);
	curr_scope = curr_scope->parent;
	delete new_scope;
	return ret;
}
ehretval_t *call_function_args(ehfm_t *f, const ehcontext_t context, const ehcontext_t newcontext, const int nargs, ehretval_t *args) {
	ehretval_t *ret = NULL;
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
	// create new scope
	ehscope_t *new_scope = new ehscope_t;
	new_scope->parent = curr_scope;
	// set parameters as necessary
	for(int i = 0; i < nargs; i++) {
		ehvar_t *var = new ehvar_t;
		var->name = f->args[i].name;
		var->scope = new_scope;
		var->value = eh_execute(&args[i], context);
		// if it's a reference, dereference it
		if(var->value->type == reference_e) {
			ehretval_t *tmp = var->value;
			var->value = var->value->referenceval;
			delete tmp;
		} else {
			var->value = var->value->share();
		}
		insert_variable(var);
	}
	// functions get their own scope (not incremented before because execution 
	// of arguments needs parent scope)
	curr_scope = new_scope;
	// set new context (only useful for methods)
	ret = eh_execute(f->code, newcontext);
	returning = false;
	
	remove_scope(curr_scope);
	curr_scope = curr_scope->parent;
	delete new_scope;
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
	ehvar_t *newmember = new ehvar_t;
	// copy the whole thing over
	newmember->name = classmember->name;
	newmember->attribute = classmember->attribute;
	// modify this pointer
	if(!strcmp(newmember->name, "this")) {
		newmember->value = new ehretval_t;
		newmember->value->type = object_e;
		newmember->value->objectval = classobj;
	} else if(classmember->attribute.isstatic == static_e) {
		newmember->value = classmember->value->reference(&(classmember->value));
	} else {
		newmember->value = classmember->value->share();
	}
	newmember->next = classobj->members[i];
	classobj->members[i] = newmember;
	return;
}
void class_insert(ehvar_t **classarr, const ehretval_t *in, ehcontext_t context) {
	// insert a member into a class
	ehretval_t *value;

	// rely on standard layout of the input ehretval_t
	ehretval_t *attribute_v = eh_execute(in->opval->paras[0], context);
	memberattribute_t attribute = attribute_v->attributestrval;
	attribute_v->dec_rc();
	char *name = in->opval->paras[1]->stringval;

	// decide what we got
	switch(in->opval->nparas) {
		case 2: // non-set property: null
			value = new ehretval_t(null_e);
			break;
		case 3: // set property
			value = eh_execute(in->opval->paras[2], context);
			break;
		case 4: // method
			value = new ehretval_t(func_e);
			value->funcval = new ehfm_t;
			value->funcval->type = user_e;
			value->funcval->code = in->opval->paras[3];
			make_arglist(&value->funcval->argcount, &value->funcval->args, 
				in->opval->paras[2]);
			break;
	}
	class_insert_retval(classarr, name, attribute, value);
}
ehvar_t *class_insert_retval(
	ehvar_t **classarr,
	const char *name,
	memberattribute_t attribute,
	ehretval_t *value
) {
	// insert a member into a class

	ehvar_t *member = new ehvar_t;
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
ehvar_t *class_getmember(const ehobj_t *classobj, const char *name, ehcontext_t context) {
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
ehretval_t *class_get(const ehobj_t *classobj, const char *name, ehcontext_t context) {
	ehretval_t *ret = NULL;

	ehvar_t *curr = class_getmember(classobj, name, context);
	if(curr != NULL) {
		ret = curr->value;
	}
	return ret;
}
ehretval_t **object_access(
	ehretval_t *operand1,
	ehretval_t *index,
	ehcontext_t context,
	int token
) {
	ehretval_t **ret = NULL;
	ehvar_t *var;
	ehvar_t *member;
	ehobj_t *object;
	memberattribute_t attribute;

	var = get_variable(operand1->stringval, curr_scope, context, T_LVALUE_GET);
	if(var == NULL) {
		eh_error("cannot access member of nonexistent variable", eerror_e);
		return NULL;
	}
	ehretval_t *label = eh_execute(index, context);

	switch(var->value->type) {
		case array_e:
			member = array_getmember(var->value->arrayval, label);
			// if there is no member yet and we are
			// setting, insert it with a null value
			if(member == NULL) {
				if(token == T_LVALUE_SET) {
					ehretval_t *val = new ehretval_t(null_e);
					member = array_insert_retval(
						var->value->arrayval, label, val
					);
					ret = &member->value;
				}
				// else use default return value
			} else {
				ret = &member->value;
			}
			break;
		case object_e:
			if(label->type != string_e) {
				eh_error_type("object member label", label->type, eerror_e);
				return NULL;
			}
			object = var->value->objectval;
		
			member = class_getmember(object, label->stringval, context);
			if(member == NULL) {
				// add new member if we're setting
				if(token == T_LVALUE_SET) {
					ehretval_t *val = new ehretval_t(null_e);
					// default is public, non-static, non-constant
					attribute.visibility = public_e;
					attribute.isstatic = nonstatic_e;
					attribute.isconst = nonconst_e;
					member = class_insert_retval(
						object->members, label->stringval, attribute, val
					);
				} else {
					eh_error_unknown(
						"object member", label->stringval, eerror_e
					);
					return NULL;
				}
			}
			// respect const specifier
			if(member->attribute.isconst == const_e) {
				if(token == T_LVALUE_SET) {
					eh_error("Attempt to write to constant variable", eerror_e);
					return NULL;
				} else {
					ret = &(member->value);
				}
			} else {
				ret = &(member->value);
			}
			newcontext = object;
			break;
		default:
			ehretval_t *ref = new ehretval_t(attribute_e);
			ref->referenceval = var->value;
			ret = new ehretval_t *;
			*ret = ref;
			break;
	}
	return ret;
}
ehretval_t **colon_access(
	ehretval_t *operand1,
	ehretval_t *index,
	ehcontext_t context,
	int token
) {
	ehretval_t **ret = NULL;
	memberattribute_t attribute;

	ehretval_t *label = eh_execute(index, context);
	if(label->type != string_e) {
		eh_error_type("object member label", label->type, eerror_e);
		return ret;
	}

	if(operand1->type != string_e) {
		eh_error_type("class access", operand1->type, efatal_e);
		return ret;
	}
	ehclass_t *classobj = get_class(operand1->stringval);
	if(classobj == NULL) {
		eh_error_unknown("class", operand1->stringval, efatal_e);
		return ret;
	}
	ehvar_t *member = class_getmember(&classobj->obj, label->stringval, context);
	if(member == NULL) {
		// add new, null member if we're setting
		if(token == T_LVALUE_SET) {
			ehretval_t *val = new ehretval_t(null_e);
			// default is public, non-static, non-constant
			attribute.visibility = public_e;
			attribute.isstatic = nonstatic_e;
			attribute.isconst = nonconst_e;
			member = class_insert_retval(
				classobj->obj.members, label->stringval, attribute, val
			);
		}
		else {
			eh_error_unknown("object member", label->stringval, eerror_e);
			return ret;
		}
	}
	// respect const specifier
	if(member->attribute.isconst == const_e) {
		if(token == T_LVALUE_SET) {
			eh_error("Attempt to write to constant variable", eerror_e);
			return NULL;
		} else {
			ret = &(member->value);
		}
	} else {
		ret = &(member->value);
	}
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
ehretval_t *eh_cast(const type_enum type, ehretval_t *in) {
	ehretval_t *ret = NULL;
	switch(type) {
// macro for the common case
#define EH_CAST_CASE(vtype) case vtype ## _e: \
	ret = eh_xto ## vtype (in); \
	break;
		EH_CAST_CASE(int)
		EH_CAST_CASE(string)
		EH_CAST_CASE(float)
		EH_CAST_CASE(range)
		EH_CAST_CASE(array)
#undef EH_CAST_CASE
		case bool_e:
			ret = new ehretval_t(eh_xtobool(in));
			break;
		default:
			eh_error_type("typecast", type, eerror_e);
			break;
	}
	return ret;
}

#define CASTERROR(totype) do { \
	eh_error_type("typecast to " #totype, in->type, enotice_e); \
	return NULL; \
} while(0)
#define CASTERROR_KNOWN(totype, vtype) do { \
	eh_error_type("typecast to " #totype, vtype ## _e, enotice_e); \
	return NULL; \
} while(0)

/* Casts between specific pairs of types */
ehretval_t *eh_stringtoint(const char *const in) {
	char *endptr;
	ehretval_t *ret = new ehretval_t(int_e);
	ret->intval = strtol(in, &endptr, 0);
	// If in == endptr, strtol read no digits and there was no conversion.
	if(in == endptr) {
		delete ret;
		CASTERROR_KNOWN(int, string);
	}
	return ret;
}
ehretval_t *eh_stringtofloat(const char *const in) {
	char *endptr;
	ehretval_t *ret = new ehretval_t(float_e);
	ret->floatval = strtof(in, &endptr);
	// If in == endptr, strtof read no digits and there was no conversion.
	if(in == endptr) {
		delete ret;
		CASTERROR_KNOWN(float, string);
	}
	return ret;
}
char *eh_inttostring(const int in) {
	// INT_MAX has 10 decimal digits on this computer, so 12 (including sign and 
	// null terminator) should suffice for the result string
	char *buffer = new char[12];
	snprintf(buffer, 12, "%d", in);

	return buffer;
}
char *eh_floattostring(const float in) {
	char *buffer = new char[12];
	snprintf(buffer, 12, "%f", in);

	return buffer;
}
char *eh_rangetostring(const ehrange_t *const range) {
	char *buffer = new char[27];
	snprintf(buffer, 27, "%d to %d", range->min, range->max);
	
	return buffer;
}
ehvar_t **eh_rangetoarray(const ehrange_t *const range) {
	ehretval_t *index;
	ehretval_t *member;

	ehvar_t **ret = new ehvar_t *[VARTABLE_S]();
	index = new ehretval_t(0);
	member = new ehretval_t(range->min);
	array_insert_retval(ret, index, member);
	index = new ehretval_t(1);
	member = new ehretval_t(range->max);
	array_insert_retval(ret, index, member);

	return ret;
}
ehretval_t *eh_stringtorange(const char *const in) {
	// attempt to find two integers in the string
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
ehretval_t *eh_xtoint(ehretval_t *in) {
	ehretval_t *ret = NULL;
	switch(in->type) {
		case int_e:
			ret = in;
			break;
		case string_e:
			ret = eh_stringtoint(in->stringval);
			break;
		case bool_e:
			ret = new ehretval_t(int_e);
			if(in->boolval) {
				ret->intval = 1;
			} else {
				ret->intval = 0;
			}
			break;
		case null_e:
			ret = new ehretval_t(0);
			break;
		case float_e:
			ret = new ehretval_t((int) in->floatval);
			break;
		default:
			CASTERROR(int);
			break;
	}
	return ret;
}
ehretval_t *eh_xtostring(ehretval_t *in) {
	ehretval_t *ret = NULL;
	switch(in->type) {
		case string_e:
			ret = in;
			break;
		case int_e:
			ret = new ehretval_t(eh_inttostring(in->intval));
			break;
		case null_e:
			// empty string
			ret = new ehretval_t(string_e);
			ret->stringval = new char[1];
			ret->stringval[0] = '\0';
			break;
		case bool_e:
			ret = new ehretval_t(string_e);
			if(in->boolval) {
				ret->stringval = new char[5];
				strcpy(ret->stringval, "true");
			}
			else {
				ret->stringval = new char[6];
				strcpy(ret->stringval, "false");
			}
			break;
		case float_e:
			ret = new ehretval_t(eh_floattostring(in->floatval));
			break;
		case range_e:
			ret = new ehretval_t(eh_rangetostring(in->rangeval));
			break;
		case array_e: // Should implode the array
		case object_e: // Should call __toString-type method
		case func_e: // Can't think of anything useful
		default:
			CASTERROR(string);
			break;
	}
	return ret;
}
bool eh_xtobool(ehretval_t *in) {
	// convert an arbitrary variable to a bool
	switch(EH_TYPE(in)) {
		case bool_e:
			return in->boolval;
		case int_e:
			return (in->intval != 0);
		case string_e:
			return (in->stringval[0] != '\0');
		case array_e:
			// empty arrays should return false
			return (array_count(in->arrayval) != 0);
		case range_e:
			// range of length zero is false, everything else is true
			return (in->rangeval->min == in->rangeval->max);
		case object_e:
		case func_e:
			// objects and functions are true if they exist
			return true;
		default:
			// other types are always false
			return false;
	}
	return false;
}
ehretval_t *eh_xtofloat(ehretval_t *in) {
	ehretval_t *ret = NULL;
	switch(in->type) {
		case float_e:
			ret = in;
			break;
		case int_e:
			ret = new ehretval_t((float) in->intval);
			break;
		case string_e:
			ret = eh_stringtofloat(in->stringval);
			break;
		case bool_e:
			if(in->boolval) {
				ret = new ehretval_t(1.0);
			} else {
				ret = new ehretval_t(0.0);
			}
			break;
		case null_e:
			ret = new ehretval_t(0.0);
			break;
		default:
			CASTERROR(float);
			break;
	}
	return ret;
}
ehretval_t *eh_xtorange(ehretval_t *in) {
	ehretval_t *ret = NULL;
	switch(in->type) {
		case range_e:
			ret = in;
			break;
		case string_e:
			ret = eh_stringtorange(in->stringval);
			break;
		case int_e:
			ret = eh_make_range(in->intval, in->intval);
			break;
		case float_e:
			ret = eh_make_range((int) in->floatval, (int) in->floatval);
			break;
		case bool_e:
			ret = eh_make_range(0, (int) in->boolval);
			break;
		case null_e:
			ret = eh_make_range(0, 0);
			break;
		default:
			CASTERROR(range);
			break;
	}
	return ret;
}
ehretval_t *eh_xtoarray(ehretval_t *in) {
	ehretval_t *ret = NULL;
	ehretval_t index(0);
	switch(in->type) {
		case array_e:
			ret = in;
			break;
		case range_e:
			ret = new ehretval_t(array_e);
			ret->arrayval = eh_rangetoarray(in->rangeval);
			break;
		case int_e:
		case bool_e:
		case string_e:
		case func_e:
		case null_e:
		case object_e:
			// create an array with just this variable in it
			ret = new ehretval_t(array_e);
			ret->arrayval = new ehvar_t *[VARTABLE_S]();
			array_insert_retval(ret->arrayval, &index, in);
			break;
		default:
			CASTERROR(array);
			break;
	}
	return ret;
}
static inline bool eh_floatequals(float infloat, ehretval_t *operand2) {
	ehretval_t *operand = eh_xtoint(operand2);
	// checks whether a float equals an int. C handles this correctly.
	if(operand->type != int_e) {
		return false;
	}
	bool result = (infloat == operand->intval);
	operand->dec_rc();
	return result;
}
ehretval_t *eh_looseequals(ehretval_t *operand1, ehretval_t *operand2) {
	ehretval_t *ret;
	// first try strict comparison
	if(eh_strictequals(operand1, operand2)) {
		ret = new ehretval_t(true);
	} else if(operand1->type == float_e) {
		ret = new ehretval_t(eh_floatequals(operand1->floatval, operand2));
	} else if(operand2->type == float_e) {
		ret = new ehretval_t(eh_floatequals(operand2->floatval, operand1));
	} else {
		operand1 = eh_xtoint(operand1);
		operand2 = eh_xtoint(operand2);
		if(EH_TYPE(operand1) == int_e && EH_TYPE(operand2) == int_e) {
			ret = new ehretval_t(operand1->intval == operand2->intval);
		} else {
			ret = NULL;
		}
	}
	return ret;
}
bool eh_strictequals(ehretval_t *operand1, ehretval_t *operand2) {
	if(operand1->type != operand2->type) {
		// strict comparison between different types always returns false
		return false;
	}
	switch(operand1->type) {
		case int_e:
			return (operand1->intval == operand2->intval);
		case string_e:
			return !strcmp(operand1->stringval, operand2->stringval);
		case bool_e:
			return (operand1->boolval == operand2->boolval);
		case null_e:
			// null always equals null
			return true;
		case float_e:
			return (operand1->floatval == operand2->floatval);
		case range_e:
			return (operand1->rangeval->min == operand2->rangeval->min)
				&& (operand1->rangeval->max == operand2->rangeval->max);
		default:
			// TODO: array comparison
			return false;
	}
	return false;
}
/*
 * Arrays
 */
void array_insert(ehvar_t **array, ehretval_t *in, int place, ehcontext_t context) {
	unsigned int vhash;
	ehretval_t *var;

	// new array member
	ehvar_t *member = new ehvar_t;

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
		const ehretval_t *label = eh_execute(in->opval->paras[0], context);
		switch(label->type) {
			case int_e:
				vhash = label->intval % VARTABLE_S;
				member->indextype = int_e;
				member->index = label->intval;
				break;
			case string_e:
				vhash = hash(label->stringval, 0);
				member->indextype = string_e;
				member->name = label->stringval;
				break;
			default:
				eh_error_type("array member label", label->type, enotice_e);
				delete member;
				return;
		}
		var = eh_execute(in->opval->paras[1], context);
	}

	// create array member
	member->value = new ehretval_t;
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
					delete (*currptr);
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
					delete (*currptr);
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
ehvar_t *array_insert_retval(ehvar_t **array, ehretval_t *index, ehretval_t *value) {
	// Inserts a member into an array. 
	// Assumes that the member is not yet present in the array.
	unsigned int vhash;

	ehvar_t *const newvar = new ehvar_t;
	newvar->indextype = index->type;
	switch(index->type) {
		case int_e:
			vhash = index->intval % VARTABLE_S;
			newvar->index = index->intval;
			break;
		case string_e:
			vhash = hash(index->stringval, 0);
			newvar->name = index->stringval;
			break;
		default:
			eh_error_type("array index", index->type, enotice_e);
			delete newvar;
			return NULL;
	}
	newvar->next = array[vhash];
	array[vhash] = newvar;
	// increase refcount?
	newvar->value = value;
	return newvar;
}
ehvar_t *array_getmember(ehvar_t **array, ehretval_t *index) {
	ehvar_t *curr;
	unsigned int vhash;

	switch(index->type) {
		case int_e:
			vhash = index->intval % VARTABLE_S;
			break;
		case string_e:
			vhash = hash(index->stringval, 0);
			break;
		default:
			eh_error_type("array index", index->type, enotice_e);
			return NULL;
	}
	curr = array[vhash];
	switch(index->type) {
		case int_e:
			for( ; curr != NULL; curr = curr->next) {
				if(curr->indextype == int_e && curr->index == index->intval) {
					return curr;
				}
			}
			break;
		case string_e:
			for( ; curr != NULL; curr = curr->next) {
				if(curr->indextype == string_e 
				  && !strcmp(curr->name, index->stringval)) {
					return curr;
				}
			}
			break;
		default: // to keep compiler happy (issues caught by previous switch)
			break;
	}
	return NULL;
}
ehretval_t *array_get(ehvar_t **array, ehretval_t *index) {
	const ehvar_t *curr = array_getmember(array, index);
	if(curr == NULL) {
		return NULL;
	} else {
		// should this increase the refcount?
		return curr->value;
	}
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
static ehretval_t *int_arrow_get(ehretval_t *operand1, ehretval_t *operand2) {
	ehretval_t *ret = NULL;
	// "array" access to integer returns the nth bit of the integer; for example 
	// (assuming sizeof(int) == 32), (2 -> 30) == 1, (2 -> 31) == 0
	if(operand2->type != int_e) {
		eh_error_type("bitwise access to integer", operand2->type, enotice_e);
		return ret;
	}
	if(operand2->intval >= (int) sizeof(int) * 8) {
		eh_error_int("Identifier too large for bitwise access to integer", 	
			operand2->intval, enotice_e);
		return ret;
	}
	// get mask
	int mask = 1 << (sizeof(int) * 8 - 1);
	mask >>= operand2->intval;
	// apply mask
	ret = new ehretval_t((int) (operand1->intval & mask) >> (sizeof(int) * 8 - 1 - mask));
	return ret;
}
static ehretval_t *string_arrow_get(ehretval_t *operand1, ehretval_t *operand2) {
	ehretval_t *ret = NULL;

	// "array" access to string returns integer representing nth character.
	// In the future, perhaps replace this with a char datatype or with a 
	// "shortstring" datatype representing strings up to 3 or even 4 characters 
	// long
	if(operand2->type != int_e) {
		eh_error_type("character access to string", operand2->type, enotice_e);
		return ret;
	}
	int count = strlen(operand1->stringval);
	if(operand2->intval >= count) {
		eh_error_int("Identifier too large for character access to string", 
			operand2->intval, enotice_e);
		return ret;
	}
	// get the nth character
	ret = new ehretval_t((int) operand1->stringval[operand2->intval]);
	return ret;
}
static ehretval_t *range_arrow_get(ehretval_t *range, ehretval_t *accessor) {
	ehretval_t *ret = NULL;
	if(accessor->type != int_e) {
		eh_error_type("arrow access to range", accessor->type, enotice_e);
		return ret;
	}
	switch(accessor->intval) {
		case 0:
			ret = new ehretval_t(range->rangeval->min);
			break;
		case 1:
			ret = new ehretval_t(range->rangeval->max);
			break;
		default:
			eh_error_int("invalid range accessor", accessor->intval, enotice_e);
			break;
	}
	return ret;
}
static void int_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue) {
	if(index->type != int_e) {
		eh_error_type("bitwise access to integer", index->type, enotice_e);
		return;
	}
	if(index->intval < 0 || (unsigned) index->intval >= sizeof(int) * 8) {
		eh_error_int("Identifier too large for bitwise access to integer", 
			index->intval, enotice_e);
		return;
	}
	// get mask
	int mask = (1 << (sizeof(int) * 8 - 1)) >> index->intval;
	if(eh_xtobool(rvalue)) {
		input->intval |= mask;
	} else {
		mask = ~mask;
		input->intval &= mask;
	}
	return;
}
static void string_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue) {
	if(rvalue->type != int_e) {
		eh_error_type("character access to string", rvalue->type, enotice_e);
		return;
	}
	if(index->type != int_e) {
		eh_error_type("setting a character in a string", index->type, enotice_e);
		return;
	}
	int count = strlen(input->stringval);
	if(index->intval >= count) {
		eh_error_int("Identifier too large for character access to string", 
			index->intval, enotice_e);
		return;
	}
	// get the nth character
	input->stringval[index->intval] = rvalue->intval;
	return;
}
static void range_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue) {
	if(rvalue->type != int_e) {
		eh_error_type("arrow access to range", rvalue->type, enotice_e);
	} else if(index->type != int_e) {
		eh_error_type("arrow access to range", index->type, enotice_e);
	} else switch(index->intval) {
		case 0:
			input->rangeval->min = rvalue->intval;
			break;
		case 1:
			input->rangeval->max = rvalue->intval;
			break;
		default:
			eh_error_int("invalid range accessor", index->intval, enotice_e);
			break;
	}
	return;
}
/*
 * Other types
 */
ehretval_t *eh_make_range(const int min, const int max) {
	ehretval_t *ret = new ehretval_t;
	ret->type = range_e;
	ret->rangeval = new ehrange_t;
	ret->rangeval->min = min;
	ret->rangeval->max = max;
	return ret;
}
static inline int count_nodes(ehretval_t *node) {
	// count a list like an argument list. Assumes correct layout.
	int i = 0;
	for(ehretval_t *tmp = node; 
		tmp->opval->nparas != 0; 
		tmp = tmp->opval->paras[0], i++
	);
	return i;

}
/*
 * Command line arguments
 */
void eh_setarg(int argc, char **argv) {
	// insert argc
	ehvar_t *argc_v = new ehvar_t;
	argc_v->value = new ehretval_t(int_e);
	// global scope
	argc_v->scope = global_scope;
	argc_v->name = "argc";
	// argc - 1, because argv[0] is ehi itself
	argc_v->value->intval = argc - 1;
	insert_variable(argc_v);

	// insert argv
	ehvar_t *argv_v = new ehvar_t;
	argv_v->value = new ehretval_t(array_e);
	argv_v->scope = global_scope;
	argv_v->name = "argv";
	argv_v->value->arrayval = new ehvar_t *[VARTABLE_S]();

	// all members of argv are strings
	for(int i = 1; i < argc; i++) {
		ehretval_t index(i - 1);
		ehretval_t *ret = new ehretval_t(argv[i]);
		array_insert_retval(argv_v->value->arrayval, &index, ret);
	}
	insert_variable(argv_v);
}
/*
 * Commands
 */
ehcmd_t *get_command(const char *name) {
	const unsigned int vhash = hash(name, 0);
	
	for(ehcmd_bucket_t *curr = cmdtable[vhash]; curr != NULL; curr = curr->next) {
		if(!strcmp(curr->cmd.name, name)) {
			return &curr->cmd;
		}
	}
	return NULL;
}
void insert_command(const ehcmd_t cmd) {
	const unsigned int vhash = hash(cmd.name, 0);
	ehcmd_bucket_t *bucket = new ehcmd_bucket_t;
	bucket->cmd = cmd;
	bucket->next = cmdtable[vhash];
	cmdtable[vhash] = bucket;
}
void redirect_command(const char *redirect, const char *target) {
	ehcmd_t *targetcmd = get_command(target);
	if(targetcmd == NULL) {
		eh_error("Unknown redirect target", eerror_e);
	}
	ehcmd_t newcmd;
	newcmd.name = redirect;
	newcmd.code = targetcmd->code;
	insert_command(newcmd);
}
