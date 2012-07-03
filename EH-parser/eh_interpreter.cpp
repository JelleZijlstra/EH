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

// library functions supported by ehi
typedef struct ehlf_listentry_t {
	const char *name;
	ehlibfunc_t code;
} ehlf_listentry_t;
#define LIBFUNCENTRY(f) {#f, ehlf_ ## f},
ehlf_listentry_t libfuncs[] = {
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
	LIBFUNCENTRY(log)
	LIBFUNCENTRY(eval)
	{NULL, NULL}
};

typedef struct ehlc_listentry_t {
	const char *name;
	ehlibclass_t info;
} ehlc_listentry_t;
#define LIBCLASSENTRY(c) { #c, {ehlc_new_ ## c, ehlc_l_ ## c }},
ehlc_listentry_t libclasses[] = {
	LIBCLASSENTRY(CountClass)
	LIBCLASSENTRY(File)
	{NULL, {NULL, NULL}}
};

typedef struct ehcmd_listentry_t {
	const char *name;
	ehcmd_t cmd;
} ehcmd_listentry_t;
#define LIBCMDENTRY(c) { #c, ehlcmd_ ## c },
ehcmd_listentry_t libcmds[] = {
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

#define DEC_RC(ret) (((ret) == NULL) ? (void)0 : (ret)->dec_rc())

static inline int count_nodes(ehretval_t *node);

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
			eh_error_types(#operator, EH_TYPE(operand1), EH_TYPE(operand2), eerror_e); \
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
	} else if(continuing) { \
		continuing = 0; \
		continue; \
	} \
	}

/*
 * Functions executed before and after the program itself is executed.
 */
EHI::EHI() : eval_parser(NULL), inloop(0), breaking(0), continuing(0), cmdtable(), arrow_access_curr(new ehretval_t(attribute_e)), returning(false), global_object(NULL) {
		eh_init();
	}
void EHI::eh_init(void) {
	global_object = new ehobj_t;
	global_object->classname = "AnonymousClass";
	global_object->parent = NULL;
	
	for(int i = 0; libfuncs[i].code != NULL; i++) {
		ehmember_t *func = new ehmember_t();
		func->value = new ehretval_t(func_e);
		func->value->funcval = new ehobj_t;
		func->value->funcval->parent = global_object;
		func->value->funcval->classname = "Closure";
		ehfm_t *f = new ehfm_t(lib_e);
		func->value->funcval->function = f;
		f->libfunc_pointer = libfuncs[i].code;
		// other fields are irrelevant
		global_object->insert(libfuncs[i].name, func);
	}
	for(int i = 0; libclasses[i].name != NULL; i++) {
		ehobj_t *newclass = new ehobj_t;
		newclass->parent = NULL;
		newclass->classname = libclasses[i].name;
		newclass->constructor = libclasses[i].info.constructor;
		ehlm_listentry_t *members = libclasses[i].info.members;
		// attributes for library methods
		memberattribute_t attributes;
		attributes.visibility = public_e;
		attributes.isstatic = nonstatic_e;
		attributes.isconst = nonconst_e;
		for(int i = 0; members[i].name != NULL; i++) {
			ehmember_t *func = new ehmember_t(attributes);
			func->value = new ehretval_t(func_e);
			func->value->funcval = new ehobj_t;
			func->value->funcval->parent = newclass;
			func->value->funcval->classname = "Closure";
			ehfm_t *f = new ehfm_t(libmethod_e);
			func->value->funcval->function = f;
			f->libmethod_pointer = members[i].func;
			newclass->insert(members[i].name, func);
		}
		ehmember_t *member = new ehmember_t(attributes);
		member->value = new ehretval_t(newclass);
		global_object->insert(newclass->classname, member);
	}
	for(int i = 0; libcmds[i].name != NULL; i++) {
		insert_command(libcmds[i].name, libcmds[i].cmd);
	}
	for(int i = 0; libredirs[i][0] != NULL; i++) {
		redirect_command(libredirs[i][0], libredirs[i][1]);
	}
	// insert reference to global object
	memberattribute_t attributes;
	attributes.visibility = public_e;
	attributes.isconst = const_e;
	attributes.isstatic = nonstatic_e;
	ehmember_t *global = new ehmember_t(attributes);
	global->value = new ehretval_t(global_object);
	global_object->insert("global", global);
	return;
}
void EHI::eh_exit(void) {
	return;
}

/*
 * Main execution function
 */
ehretval_t *EHI::eh_execute(ehretval_t *node, const ehcontext_t context) {
	// variables used
	ehretval_t *ret, *operand1, *operand2;
	bool b1, b2;
	// default
	ret = NULL;

	// empty statements produce a null node
	if(node == NULL) {
		return ret;
	}
	try {
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
					} else {
						// check for empty statement; this means that the last
						// actual statement in a function is returned
						ehretval_t *newNode = node->opval->paras[1];
						if(EH_TYPE(newNode) == op_e && newNode->opval->op == T_SEPARATOR && newNode->opval->nparas == 0) {
							return ret;
						} else {
							ret = eh_execute(newNode, context);
						}
					}
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
					ret = eh_op_new(node->opval->paras, context);
					break;
			/*
			 * Object definitions
			 */
				case T_FUNC: // function definition
					ret = eh_op_declareclosure(node->opval->paras, context);
					break;
				case T_CLASS: // class declaration
					ret = eh_op_declareclass(node->opval, context);
					break;
				case T_CLASSMEMBER:
					eh_op_classmember(node->opval, context);
					break;
				case T_INHERIT:
					eh_op_inherit(node->opval->paras, context);
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
					ret = eh_op_lvalue(node->opval, context);
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
								eh_error_type("-- operator", EH_TYPE(operand1), eerror_e);
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
								eh_error_type("++ operator", EH_TYPE(operand1), eerror_e);
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
	} catch(int) {
		// ignore all exceptions
	}
	return ret;
}
/*
 * Opnode execution helpers
 */
ehretval_t *EHI::eh_op_command(const char *name, ehretval_t *node, ehcontext_t context) {
	ehretval_t *value_r;
	// count for simple parameters
	int count = 0;
	// we're making an array of parameters
	eharray_t paras;
	// loop through the paras given
	for( ; node->opval->nparas != 0; node = node->opval->paras[1]) {
		ehretval_t *node2 = node->opval->paras[0];
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
						char index[2];
						index[0] = node2->stringval[i];
						index[1] = '\0';
						paras.string_indices[index] = value_r;
					}
					break;
				case T_LONGPARA:
				{
					// long-form paras
					char *index = node2->opval->paras[0]->stringval;
					if(node2->opval->nparas == 1) {
						paras.string_indices[index] = new ehretval_t(true);
					} else {
						paras.string_indices[index] = eh_execute(node2->opval->paras[1], context);
					}
					break;
				}
				case T_REDIRECT:
					paras.string_indices[">"] = eh_execute(node2->opval->paras[0], context);
					break;
				case '}':
					paras.string_indices["}"] = eh_execute(node2->opval->paras[0], context);
					break;
				default: // non-named parameters with an expression
					// non-named parameters
					paras.int_indices[count] = eh_execute(node2, context);
					count++;
					break;
			}
		} else {
			// non-named parameters
			paras.int_indices[count] = node2;
			count++;
		}
	}
	// insert indicator that this is an EH-PHP command
	paras.string_indices["_ehphp"] = new ehretval_t(true);
	// get the command to execute
	const ehcmd_t libcmd = get_command(name);
	ehretval_t *ret;
	if(libcmd != NULL) {
		ret = libcmd(&paras);
	} else {
		ret = execute_cmd(name, &paras);
	}
	// we're not returning anymore
	returning = false;
	return ret;
}
ehretval_t *EHI::eh_op_for(opnode_t *op, ehcontext_t context) {
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
		if(EH_TYPE(count_r) != int_e) {
			eh_error_type("count", EH_TYPE(count_r), eerror_e);
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
	} else {
		// "for 5 count i; do stuff; endfor" construct
		char *name = op->paras[1]->stringval;
		// this should perhaps create a new variable, or only overwrite variables in the current scope
		ehmember_t *var = context->get_recursive(name, context, T_LVALUE_SET);
		// if we do T_LVALUE_SET, get_recursive never returns NULL
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
ehretval_t *EHI::eh_op_while(ehretval_t **paras, ehcontext_t context) {
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
ehretval_t *EHI::eh_op_as(opnode_t *op, ehcontext_t context) {
	ehretval_t *ret = NULL;

	// get the object to be looped through and check its type
	ehretval_t *object = eh_execute(op->paras[0], context);
	if(EH_TYPE(object) != array_e && EH_TYPE(object) != object_e) {
		eh_error_type("for ... as operator", EH_TYPE(object), enotice_e);
		return ret;
	}
	// increment loop count
	inloop++;
	// establish variables
	char *membername;
	ehmember_t *membervar;
	char *indexname;
	ehmember_t *indexvar = NULL;
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
	membervar = context->get_recursive(membername, context, T_LVALUE_SET);
	if(indexname != NULL) {
		indexvar = context->get_recursive(indexname, context, T_LVALUE_SET);
	}
	if(object->type == object_e) {
		// object index is always a string
		if(indexname != NULL) {
			if(indexvar->value == NULL) {
				indexvar->value = new ehretval_t(string_e);
			}
		}
		// check whether we're allowed to access private things
		const bool doprivate = ehcontext_compare(object->objectval, context);
		OBJECT_FOR_EACH(object->objectval, curr) {
			// ignore private
			if(!doprivate && curr->second->attribute.visibility == private_e) {
				continue;
			}
			membervar->value = curr->second->value->share();
			if(indexname) {
				// need the strdup here because currmember->name is const
				// and a string_e is not. Perhaps solve this instead by
				// creating a new cstring_e type?
				indexvar->value->stringval = strdup(curr->first.c_str());
			}
			ret = eh_execute(code, context);
			LOOPCHECKS;
		
		}
	} else {
		// arrays
		eharray_t *array = object->arrayval;
		if(indexname) {
			if(indexvar->value == NULL) {
				indexvar->value = new ehretval_t(int_e);
			}
			indexvar->value->type = int_e;
		}
		ARRAY_FOR_EACH_INT(array, i) {
			if(indexname) {
				indexvar->value->intval = i->first;
			}
			membervar->value = i->second;
			ret = eh_execute(code, context);
			LOOPCHECKS;
		}
		if(indexname) {
			indexvar->value->type = string_e;
		}
		ARRAY_FOR_EACH_STRING(array, i) {
			if(indexname) {
				indexvar->value->stringval = (char *)i->first.c_str();
			}
			membervar->value = i->second;
			ret = eh_execute(code, context);
			LOOPCHECKS;		
		}
	}
	inloop--;
	return ret;
}
ehretval_t *EHI::eh_op_new(ehretval_t **paras, ehcontext_t context) {
	ehobj_t *classobj = this->get_class(paras[0], context);
	ehretval_t *ret = NULL;
	// get_class complains for us
	if(classobj != NULL) {
		ret = new ehretval_t(this->object_instantiate(classobj));
	}
	return ret;
}
void EHI::eh_op_inherit(ehretval_t **paras, ehcontext_t context) {
	ehobj_t *classobj = this->get_class(paras[0], context);
	if(classobj != NULL) {
		OBJECT_FOR_EACH(classobj, i) {
			context->copy_member(i, true);
		}
	}
}
void EHI::eh_op_break(opnode_t *op, ehcontext_t context) {
	int level;
	if(op->nparas == 0) {
		level = 1;
	} else {
		ehretval_t *level_v = eh_xtoint(eh_execute(op->paras[0], context));
		if(EH_TYPE(level_v) != int_e) {
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
void EHI::eh_op_continue(opnode_t *op, ehcontext_t context) {
	int level;
	if(op->nparas == 0) {
		level = 1;
	} else {
		ehretval_t *level_v = eh_xtoint(eh_execute(op->paras[0], context));
		if(EH_TYPE(level_v) != int_e) {
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
ehretval_t *EHI::eh_op_array(ehretval_t *node, ehcontext_t context) {
	ehretval_t *ret = new ehretval_t(new eharray_t);
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
ehretval_t *EHI::eh_op_anonclass(ehretval_t *node, ehcontext_t context) {
	ehretval_t *ret = new ehretval_t(new ehobj_t);
	ret->objectval->classname = "AnonClass";
	// all members are public, non-static, non-const
	memberattribute_t attributes;
	attributes.visibility = public_e;
	attributes.isconst = nonconst_e;
	attributes.isstatic = nonstatic_e;

	for( ; node->opval->nparas != 0; node = node->opval->paras[0]) {
		ehretval_t **myparas = node->opval->paras[1]->opval->paras;
		// nodes here will always have the name in para 0 and value in para 1
		ehretval_t *namev = eh_execute(myparas[0], context);
		if(EH_TYPE(namev) != string_e) {
			eh_error_type("Class member label", EH_TYPE(namev), eerror_e);
			continue;
		}
		ehretval_t *value = eh_execute(myparas[1], context);
		ret->objectval->insert_retval(namev->stringval, attributes, value);
	}
	return ret;
}
ehretval_t *EHI::eh_op_declareclosure(ehretval_t **paras, ehcontext_t context) {
	ehretval_t *ret = new ehretval_t(func_e);
	ret->funcval = new ehobj_t;
	ret->funcval->parent = context;
	ret->funcval->classname = "Closure";

	ehfm_t *f = new ehfm_t(user_e);
	ret->funcval->function = f;
	f->code = paras[1];

	// determine argument count
	f->argcount = count_nodes(paras[0]);
	// if there are no arguments, the arglist can be NULL
	if(f->argcount) {
		f->args = new eharg_t[f->argcount]();
	} else {
		f->args = NULL;
	}
	// add arguments to arglist
	int i = 0;
	for(ehretval_t *tmp = paras[0]; tmp->opval->nparas != 0; 
		tmp = tmp->opval->paras[0]) {
		f->args[i].name = tmp->opval->paras[1]->stringval;
		i++;
	}
	return ret;
}
ehretval_t *EHI::eh_op_declareclass(opnode_t *op, ehcontext_t context) {
	// process parameters
	const char *name;
	ehretval_t *code;
	if(op->nparas == 2) {
		name = op->paras[0]->stringval;
		code = op->paras[1];
	} else {
		name = "AnonymousClass";
		code = op->paras[0];
	}

	ehobj_t *classobj = new ehobj_t();
	classobj->parent = context;
	classobj->classname = name;

	// insert "this" pointer
	memberattribute_t thisattributes;
	thisattributes.visibility = private_e;
	thisattributes.isstatic = nonstatic_e;
	thisattributes.isconst = const_e;
	ehretval_t *thisvalue = new ehretval_t(classobj);
	classobj->insert_retval("this", thisattributes, thisvalue);

	eh_execute(code, classobj);
	
	// create the ehretval_t
	ehretval_t *ret = new ehretval_t(classobj);
	if(op->nparas == 2) {
		// insert variable
		ehmember_t *member = new ehmember_t();
		member->value = ret;
		context->insert(name, member);
	}
	return ret;
}
void EHI::eh_op_classmember(opnode_t *op, ehcontext_t context) {
	// rely on standard layout of the paras
	ehretval_t *attribute_v = eh_execute(op->paras[0], context);
	memberattribute_t attribute = attribute_v->attributestrval;
	char *name = op->paras[1]->stringval;

	// decide what we got
	ehretval_t *value;
	switch(op->nparas) {
		case 2: // non-set property: null
			value = NULL;
			break;
		case 3: // set property
			value = eh_execute(op->paras[2], context);
			break;
	}
	context->insert_retval(name, attribute, value);
}
ehretval_t *EHI::eh_op_switch(ehretval_t **paras, ehcontext_t context) {
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
			if(EH_TYPE(casevar) == func_e) {
				decider = call_function_args(casevar->funcval, 1, &switchvar, context);
				if(EH_TYPE(decider) != bool_e) {
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
ehretval_t *EHI::eh_op_given(ehretval_t **paras, ehcontext_t context) {
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
		if(EH_TYPE(casevar) == func_e) {
			decider = call_function_args(
				casevar->funcval, 1, &switchvar, context
			);
			if(EH_TYPE(decider) != bool_e) {
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
ehretval_t *EHI::eh_op_colon(ehretval_t **paras, ehcontext_t context) {
	ehretval_t *ret = NULL;
	ehmember_t *func;

	ehretval_t *function = eh_execute(paras[0], context);
	// operand1 will be either a string (indicating a normal function call) or a 
	// func_e (indicating a method or closure call)
	switch(EH_TYPE(function)) {
		case string_e:
			func = context->get_recursive(
				function->stringval, context, T_LVALUE_GET
			);
			if(func == NULL) {
				eh_error_unknown("function", function->stringval, eerror_e);
				return NULL;
			}
			if(EH_TYPE(func->value) != func_e) {
				eh_error_type("function call", EH_TYPE(func->value), eerror_e);
				return NULL;
			}
			ret = call_function(
				func->value->funcval, paras[1], context
			);
			break;
		case func_e:
			ret = call_function(
				function->funcval, paras[1], context
			);
			break;
		case null_e:
			// ignore null functions to prevent duplicate warnings
			return NULL;
		default:
			eh_error_type("function call", EH_TYPE(function), eerror_e);
			return NULL;
	}
	function->dec_rc();
	return ret;
}
ehretval_t *EHI::eh_op_reference(opnode_t *op, ehcontext_t context) {
	ehretval_t *var = eh_op_lvalue(op, context);
	if(var == NULL) {
		eh_error("Unable to create reference", eerror_e);
	}
	ehretval_t *ret = new ehretval_t(reference_e);
	ret->referenceval = var->reference(var);
	return ret;
}
ehretval_t *&EHI::eh_op_lvalue(opnode_t *op, ehcontext_t context) {
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
	ehretval_t *basevar = eh_execute(op->paras[0], context);
	// We need this because of code in eh_op_set checking for this. Removing
	// the check for NULL there yields a problem where $ foo->2 = 0 produces 
	// $foo = @int 0.
	switch(op->nparas) {
		case 1:
		{
			ehmember_t *var = context->get_recursive(basevar->stringval, context, op->op);
			// dereference variable
			if(var != NULL) {
				basevar->dec_rc();
				// increase refcount?
				return var->value;
			}
			/*
			 * If there is no variable of this name, and it is a
			 * simple access, we throw.
			 */
			break;
		}
		case 3:
			switch(op->paras[1]->accessorval) {
				case arrow_e:
					return object_access(basevar, op->paras[2], context, op->op);
				case doublecolon_e:
					return colon_access(basevar, op->paras[2], context, op->op);
			}
			break;
	}
	throw 0;
}
ehretval_t *EHI::eh_op_dollar(ehretval_t *node, ehcontext_t context) {
	ehretval_t *ret = eh_execute(node, context);
	if(ret == NULL) {
		return NULL;
	}
	
	ehretval_t *varname = eh_xtostring(ret);
	if(varname == NULL) {
		return NULL;
	}
	
	ehmember_t *var = context->get_recursive(
		varname->stringval, context, T_LVALUE_GET
	);
	if(var == NULL) {
		return NULL;
	} else {
		return var->value;
	}
}
void EHI::eh_op_set(ehretval_t **paras, ehcontext_t context) {
	try {
		ehretval_t *&lvalue = eh_op_lvalue(paras[0]->opval, context);
		ehretval_t *rvalue = eh_execute(paras[1], context);
		if(rvalue != NULL) {
			rvalue->inc_rc();
		}
		if(EH_TYPE(lvalue) == attribute_e) {
			// lvalue is a pointer to the variable modified, rvalue is the value set to, index is the index
			ehretval_t *index = eh_execute(paras[0]->opval->paras[2], context);
			switch(EH_TYPE(lvalue->referenceval)) {
				case int_e:
					int_arrow_set(lvalue->referenceval, index, rvalue);
					break;
				case string_e:
					string_arrow_set(lvalue->referenceval, index, rvalue);
					break;
				case range_e:
					range_arrow_set(lvalue->referenceval, index, rvalue);
					break;
				default:
					eh_error_type("arrow access", EH_TYPE(lvalue->referenceval), eerror_e);
					break;
			}
			DEC_RC(index);
		} else {
			if(EH_TYPE(rvalue) == reference_e) {
				// set new reference
				lvalue = rvalue->referenceval;
				rvalue->dec_rc();
			} else if(rvalue == NULL) {
				lvalue = NULL;
			} else if(lvalue == NULL) {
				lvalue = rvalue->share();
			} else {
				// set variable
				lvalue = lvalue->overwrite(rvalue);
			}
		}
	} catch(int) {
		// do nothing
	}
	return;
}
ehretval_t *EHI::eh_op_accessor(ehretval_t **paras, ehcontext_t context) {
	// this only gets executed for array-type int and string access
	ehretval_t *ret = NULL;
	ehretval_t *basevar = eh_execute(paras[0], context);
	ehretval_t *index = eh_execute(paras[2], context);
	switch(paras[1]->accessorval) {
		case arrow_e:
			// "array" access
			switch(EH_TYPE(basevar)) {
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
					if(basevar->arrayval->has(index)) {
						ret = basevar->arrayval->operator[](index);
					}
					break;
				case object_e:
					if(EH_TYPE(index) != string_e) {
						eh_error_type("access to object", EH_TYPE(index), eerror_e);
					} else {
						ehmember_t *member = basevar->objectval->get(index->stringval, context, T_LVALUE_GET);
						if(member == NULL) {
							eh_error_unknown(
								"object member", index->stringval, eerror_e
							);
						} else {
							ret = member->value;
						}
					}
					break;
				default:
					eh_error_type("arrow access", EH_TYPE(basevar), eerror_e);
					break;
			}
			break;
		case doublecolon_e:
			try {
				ret = colon_access(basevar, index, context, T_LVALUE_GET);
			} catch(int) {
			}
			break;
		default:
			eh_error("Unsupported accessor", efatal_e);
			break;
	}
	DEC_RC(basevar);
	DEC_RC(index);
	return ret;
}
/*
 * Functions
 */
ehretval_t *EHI::call_function(ehobj_t *obj, ehretval_t *args, ehcontext_t context) {
	// this is a wrapper for call_function_args; it parses the arguments and
	// puts them in an array
	int nargs = count_nodes(args);
	ehretval_t **new_args = new ehretval_t*[nargs]();
	
	for(int i = 0; args->opval->nparas != 0; (args = args->opval->paras[0]) && i++) {
		new_args[i] = eh_execute(args->opval->paras[1], context);
	}
	ehretval_t *ret = this->call_function_args(obj, nargs, new_args, context);
	delete[] new_args;

	return ret;
}
ehretval_t *EHI::call_function_args(ehobj_t *obj, const int nargs, ehretval_t *args[], ehcontext_t context) {
	ehretval_t *ret = NULL;
	
	ehfm_t *f = obj->function;
	if(f == NULL) {
		eh_error("Invalid object for function call", eerror_e);
		return NULL;
	}

	if(f->type == lib_e) {
		return f->libfunc_pointer(nargs, args, context, this);
	} else if(f->type == libmethod_e) {
		return f->libmethod_pointer(obj->parent, nargs, args, context, this);
	}
	// check parameter count
	if(nargs != f->argcount) {
		eh_error_argcount(f->argcount, nargs);
		return NULL;
	}
	ehobj_t *newcontext = object_instantiate(obj);
	
	// set parameters as necessary
	for(int i = 0; i < nargs; i++) {
		ehmember_t *var = new ehmember_t();
		var->value = args[i];
		if(var->value->get_type() == reference_e) {
			var->value = var->value->referenceval;
		} else {
			var->value = var->value->share();
		}
		newcontext->insert(f->args[i].name, var);
	}
	ret = eh_execute(f->code, newcontext);
	returning = false;
	
	delete newcontext;
	return ret;
}
/*
 * Classes
 */
ehobj_t *EHI::object_instantiate(ehobj_t *obj) {
	ehobj_t *ret = new ehobj_t;
	ret->classname = obj->classname;
	ret->function = obj->function;
	ret->parent = obj->parent;
	ret->real_parent = obj->real_parent;
	if(obj->function != NULL && obj->function->type == libmethod_e) {
		// insert selfptr
		ret->selfptr = obj->constructor();
	}
	
	ehretval_t *constructor = NULL;
	for(int i = 0; i < VARTABLE_S; i++) {
		OBJECT_FOR_EACH(obj, m) {
			ret->copy_member(m, false);
			if(m->first.compare("constructor") == 0) {
				constructor = m->second->value;
			}
		}
	}
	if(constructor != NULL) {
		if(EH_TYPE(constructor) != func_e) {
			eh_error_type("constructor", EH_TYPE(constructor), enotice_e);
		} else {
			call_function_args(constructor->funcval, 0, NULL, obj->parent);
		}
	}
	return ret;
}
ehretval_t *&EHI::object_access(ehretval_t *operand1, ehretval_t *index, ehcontext_t context, int token) {
	ehmember_t *member;

	ehmember_t *var = context->get_recursive(operand1->stringval, context, T_LVALUE_GET);
	if(var == NULL) {
		eh_error("cannot access member of nonexistent variable", eerror_e);
		throw 0;
	}
	ehretval_t *label = eh_execute(index, context);
	if(label == NULL) {
		throw 0;
	}

	switch(var->value->type) {
		case array_e:
			if(var->value->arrayval->has(label) or (token == T_LVALUE_SET)) {
				return var->value->arrayval->operator[](label);
			} else {
				throw 0;
			}
		case object_e:
			if(EH_TYPE(label) != string_e) {
				eh_error_type("object member label", EH_TYPE(label), eerror_e);
				throw 0;
			}
			member = var->value->objectval->get(label->stringval, context, token);
			if(member == NULL) {
				throw 0;
			} else {
				return member->value;
			}
			break;
		default:
			arrow_access_curr->type = attribute_e;
			arrow_access_curr->referenceval = var->value;
			return arrow_access_curr;
	}
	throw 0;
}
ehretval_t *&EHI::colon_access(ehretval_t *operand1, ehretval_t *index, ehcontext_t context, int token) {
	ehretval_t *label = eh_execute(index, context);
	if(EH_TYPE(label) != string_e) {
		eh_error_type("object member label", EH_TYPE(label), eerror_e);
		throw 0;
	}

	ehobj_t *classobj = this->get_class(operand1, context);
	if(classobj == NULL) {
		throw 0;
	}
	ehmember_t *member = classobj->get(label->stringval, context, token);
	if(member == NULL) {
		if(token == T_LVALUE_GET) {
			eh_error_unknown("object member", label->stringval, eerror_e);		
		}
		throw 0;
	} else {
		return member->value;
	}
}
ehobj_t *EHI::get_class(ehretval_t *code, ehcontext_t context) {
	ehretval_t *classname = this->eh_execute(code, context);
	ehobj_t *classobj;
	switch(EH_TYPE(classname)) {
		case string_e:
		{
			ehmember_t *member = context->get_recursive(classname->stringval, context, T_LVALUE_GET);
			if(member == NULL) {
				eh_error_unknown("class", classname->stringval, eerror_e);
				return NULL;
			}
			if(EH_TYPE(member->value) != object_e) {
				eh_error_type("class", EH_TYPE(member->value), eerror_e);
				return NULL;
			}
			classobj = member->value->objectval;
			break;
		}
		case object_e:
			classobj = classname->objectval;
			break;
		default:
			eh_error_type("class name", EH_TYPE(classname), eerror_e);
			return NULL;
	}
	return classobj;
}

/*
 * Arrays
 */
void EHI::array_insert(eharray_t *array, ehretval_t *in, int place, ehcontext_t context) {
	/*
	 * We'll assume we're always getting a correct ehretval_t *, referring to a
	 * T_ARRAYMEMBER token. If there is 1 parameter, that means it's a
	 * non-labeled array member, which we'll give an integer array index; if
	 * there are 2, we'll either use the integer array index or a hash of the
	 * string index.
	 */
	if(in->opval->nparas == 1) {
		// if there is no explicit key, simply use the place argument
		array->int_indices[place] = eh_execute(in->opval->paras[0], context);
	} else {
		const ehretval_t *label = eh_execute(in->opval->paras[0], context);
		ehretval_t *var = eh_execute(in->opval->paras[1], context);
		switch(EH_TYPE(label)) {
			case int_e:
				array->int_indices[label->intval] = var;
				break;
			case string_e:
				array->string_indices[label->stringval] = var;
				break;
			default:
				eh_error_type("array member label", EH_TYPE(label), enotice_e);
				break;
		}
	}
	return;
}
/*
 * Command line arguments
 */
void EHI::eh_setarg(int argc, char **argv) {
	// insert argc
	ehmember_t *argc_v = new ehmember_t;
	// argc - 1, because argv[0] is ehi itself
	argc_v->value = new ehretval_t(argc - 1);
	global_object->insert("argc", argc_v);

	// insert argv
	ehmember_t *argv_v = new ehmember_t;
	argv_v->value = new ehretval_t(new eharray_t);

	// all members of argv are strings
	for(int i = 1; i < argc; i++) {
		argv_v->value->arrayval->int_indices[i - 1] = new ehretval_t(argv[i]);
	}
	global_object->insert("argv", argv_v);
}
/*
 * Commands
 */
ehcmd_t EHI::get_command(const char *name) {
	if(this->cmdtable.count(name) == 1) {
		return this->cmdtable[name];
	} else {
		return NULL;
	}
}
void EHI::insert_command(const char *name, const ehcmd_t cmd) {
	this->cmdtable[name] = cmd;
}
void EHI::redirect_command(const char *redirect, const char *target) {
	ehcmd_t targetcmd = get_command(target);
	if(targetcmd == NULL) {
		eh_error("Unknown redirect target", eerror_e);
	}
	insert_command(redirect, targetcmd);
}

/*****************************************
 * Functions outside the EHI object.     *
 *****************************************/
/*
 * Opcode handlers.
 */
ehretval_t *eh_count(const ehretval_t *in) {
	ehretval_t *ret = new ehretval_t(int_e);
	switch(EH_TYPE(in)) {
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
			ret->intval = in->arrayval->size();
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
			eh_error_type("count operator", EH_TYPE(in), eerror_e);
			ret->type = null_e;
			break;
	}
	return ret;
}
ehretval_t *eh_op_tilde(ehretval_t *in) {
	// no const argument because it's modified below
	ehretval_t *ret = NULL;
	switch(EH_TYPE(in)) {
		// bitwise negation of a bool is just normal negation
		case bool_e:
			ret = new ehretval_t(!in->boolval);
			break;
		// else try to cast to int
		default:
			in = eh_xtoint(in);
			if(EH_TYPE(in) != int_e) {
				eh_error_type("bitwise negation", EH_TYPE(in), eerror_e);
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
	switch(EH_TYPE(in)) {
		// negation
		case bool_e:
			ret = new ehretval_t(!in->boolval);
			break;
		case float_e:
			ret = new ehretval_t(-in->floatval);
			break;
		default:
			in = eh_xtoint(in);
			if(EH_TYPE(in) != int_e) {
				eh_error_type("negation", EH_TYPE(in), eerror_e);
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

/*
 * Classes.
 */
bool ehcontext_compare(const ehcontext_t lock, const ehcontext_t key) {
	// in global context, we never have access to private stuff
	if(key == NULL) {
		return false;
	} else {
		if(strcmp(lock->classname, key->classname) == 0) {
			return true;
		} else {
			return ehcontext_compare(lock, key->parent);
		}
	}
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
	eh_error_type("typecast to " #totype, EH_TYPE(in), enotice_e); \
	return NULL; \
} while(0)
#define CASTERROR_KNOWN(totype, vtype) do { \
	eh_error_type("typecast to " #totype, vtype ## _e, enotice_e); \
	return NULL; \
} while(0)

/* Casts between specific pairs of types */
ehretval_t *eh_stringtoint(const char *const in) {
	char *endptr;
	ehretval_t *ret = new ehretval_t((int) strtol(in, &endptr, 0));
	// If in == endptr, strtol read no digits and there was no conversion.
	if(in == endptr) {
		delete ret;
		CASTERROR_KNOWN(int, string);
	}
	return ret;
}
ehretval_t *eh_stringtofloat(const char *const in) {
	char *endptr;
	ehretval_t *ret = new ehretval_t(strtof(in, &endptr));
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
eharray_t *eh_rangetoarray(const ehrange_t *const range) {
	eharray_t *ret = new eharray_t;
	ret->int_indices[0] = new ehretval_t(range->min);
	ret->int_indices[1] = new ehretval_t(range->max);
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
	switch(EH_TYPE(in)) {
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
	switch(EH_TYPE(in)) {
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
			return (in->arrayval->size() != 0);
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
	switch(EH_TYPE(in)) {
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
	switch(EH_TYPE(in)) {
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
	switch(EH_TYPE(in)) {
		case array_e:
			ret = in;
			break;
		case range_e:
			ret = new ehretval_t(eh_rangetoarray(in->rangeval));
			break;
		case int_e:
		case bool_e:
		case string_e:
		case func_e:
		case null_e:
		case object_e:
			// create an array with just this variable in it
			ret = new ehretval_t(new eharray_t);
			ret->arrayval->int_indices[0] = in;
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
	if(EH_TYPE(operand) != int_e) {
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
	} else if(EH_TYPE(operand1) == float_e) {
		ret = new ehretval_t(eh_floatequals(operand1->floatval, operand2));
	} else if(EH_TYPE(operand2) == float_e) {
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
	if(EH_TYPE(operand1) != EH_TYPE(operand2)) {
		// strict comparison between different types always returns false
		return false;
	}
	switch(EH_TYPE(operand1)) {
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
 * Variants of array access
 */
ehretval_t *int_arrow_get(ehretval_t *operand1, ehretval_t *operand2) {
	ehretval_t *ret = NULL;
	// "array" access to integer returns the nth bit of the integer; for example 
	// (assuming sizeof(int) == 32), (2 -> 30) == 1, (2 -> 31) == 0
	if(EH_TYPE(operand2) != int_e) {
		eh_error_type("bitwise access to integer", EH_TYPE(operand2), enotice_e);
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
ehretval_t *string_arrow_get(ehretval_t *operand1, ehretval_t *operand2) {
	ehretval_t *ret = NULL;

	// "array" access to string returns integer representing nth character.
	// In the future, perhaps replace this with a char datatype or with a 
	// "shortstring" datatype representing strings up to 3 or even 4 characters 
	// long
	if(EH_TYPE(operand2) != int_e) {
		eh_error_type("character access to string", EH_TYPE(operand2), enotice_e);
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
ehretval_t *range_arrow_get(ehretval_t *range, ehretval_t *accessor) {
	ehretval_t *ret = NULL;
	if(EH_TYPE(accessor) != int_e) {
		eh_error_type("arrow access to range", EH_TYPE(accessor), enotice_e);
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
void int_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue) {
	if(EH_TYPE(index) != int_e) {
		eh_error_type("bitwise access to integer", EH_TYPE(index), enotice_e);
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
void string_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue) {
	if(EH_TYPE(rvalue) != int_e) {
		eh_error_type("character access to string", EH_TYPE(rvalue), enotice_e);
		return;
	}
	if(EH_TYPE(index) != int_e) {
		eh_error_type("setting a character in a string", EH_TYPE(index), enotice_e);
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
void range_arrow_set(ehretval_t *input, ehretval_t *index, ehretval_t *rvalue) {
	if(EH_TYPE(rvalue) != int_e) {
		eh_error_type("arrow access to range", EH_TYPE(rvalue), enotice_e);
	} else if(EH_TYPE(index) != int_e) {
		eh_error_type("arrow access to range", EH_TYPE(index), enotice_e);
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
	ehretval_t *ret = new ehretval_t(new ehrange_t(min, max));
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
 * ehretval_t
 */
void ehretval_t::print() {
	switch(EH_TYPE(this)) {
		case string_e:
			printf("%s", this->stringval);
			break;
		case int_e:
			printf("%d", this->intval);
			break;
		case bool_e:
			if(this->boolval) {
				printf("(true)");
			} else {
				printf("(false)");
			}
			break;
		case null_e:
			printf("(null)");
			break;
		case float_e:
			printf("%f", this->floatval);
			break;
		case range_e:
			printf("%d to %d", this->rangeval->min, this->rangeval->max);
			break;
		default:
			eh_error_type("echo operator", EH_TYPE(this), enotice_e);
			break;
	}
	return;
}

/*
 * eharray_t
 */
ehretval_t * &eharray_t::operator[](ehretval_t *index) {
	switch(EH_TYPE(index)) {
		case int_e:
			return int_indices[index->intval];
		case string_e:
			return string_indices[index->stringval];
		default:
			eh_error_type("array index", EH_TYPE(index), enotice_e);
			throw new std::exception;
	}
}
void eharray_t::insert_retval(ehretval_t *index, ehretval_t *value) {
	// Inserts a member into an array. 
	switch(EH_TYPE(index)) {
		case int_e:
			this->int_indices[index->intval] = value;
			break;
		case string_e:
			this->string_indices[index->stringval] = value;
			break;
		default:
			eh_error_type("array index", EH_TYPE(index), enotice_e);
			break;
	}
}
ehmember_t *ehobj_t::insert_retval(const char *name, memberattribute_t attribute, ehretval_t *value) {
	if(this->has(name)) {
		eh_error("object member already set", enotice_e);
		return NULL;
	}
	// insert a member into a class
	ehmember_t *member = new ehmember_t(attribute);
	member->value = value;

	// insert into object
	members[name] = member;
	return member;
}
/*
 * ehobj_t
 */
ehmember_t *ehobj_t::get(const char *name, const ehcontext_t context, int token) {
	ehmember_t *out = NULL;
	if(this->has(name)) {
		out = this->members[name];
		if(out->attribute.visibility == private_e) {
			// check context
			if(!ehcontext_compare(this, context)) {
				out = NULL;
			}
		}
	} else {
		if(token == T_LVALUE_SET) {
			out = new ehmember_t();
			out->value = NULL;
			this->insert(name, out);
		}
	}
	if(out != NULL && token == T_LVALUE_SET && out->attribute.isconst == const_e) {
		eh_error("Attempt to write to constant variable", eerror_e);
		return NULL;
	}
	return out;
}
ehmember_t *ehobj_t::get_recursive(const char *name, ehcontext_t context, int token) {
	ehmember_t *currvar = this->get_recursive_helper(name, context);
	if(token == T_LVALUE_SET) {
		if(currvar == NULL) {
			if(!this->has(name)) {
				currvar = new ehmember_t();
				currvar->value = NULL;
				this->insert(name, currvar);
			} else {
				throw 0;
			}
		} else if(currvar->attribute.isconst == const_e) {
			eh_error("Attempt to write to constant variable", eerror_e);
			throw 0;
		}
	}
	return currvar;
}
ehmember_t *ehobj_t::get_recursive_helper(const char *name, const ehcontext_t context) {
	ehmember_t *out = NULL;
	if(this->has(name)) {
		out = this->members[name];
	}
	if(out == NULL) {
		if(this->real_parent == NULL) {
			if(this->parent != NULL) {
				out = this->parent->get_recursive_helper(name, context);
			}
		} else {
			if(this->parent != NULL && this->parent->has(name)) {
				out = this->parent->members[name];
			} else {
				out = this->real_parent->get_recursive_helper(name, context);
			}
		}
	}
	return out;
}
void ehobj_t::copy_member(obj_iterator &classmember, bool set_real_parent) {
	ehmember_t *newmember = new ehmember_t;
	newmember->attribute = classmember->second->attribute;
	// modify this pointer
	if(classmember->first.compare("this") == 0) {
		newmember->value = new ehretval_t(this);
	} else if(classmember->second->attribute.isstatic == static_e) {
		// can't share NULL
		if(classmember->second->value == NULL) {
			classmember->second->value = new ehretval_t(null_e);
		}
		newmember->value = classmember->second->value->reference(classmember->second->value);
	} else if(EH_TYPE(classmember->second->value) == func_e) {
		newmember->value = new ehretval_t(func_e);
		ehobj_t *f = new ehobj_t();
		newmember->value->funcval = f;
		f->parent = this;
		ehobj_t *oldobj = classmember->second->value->funcval;
		if(set_real_parent && oldobj->real_parent == NULL) {
			f->real_parent = oldobj->parent->parent;
		} else {
			f->real_parent = oldobj->real_parent;
		}
		f->function = oldobj->function;
		f->classname = oldobj->classname;
		f->members = oldobj->members;
	} else if(classmember->second->value == NULL) {
		newmember->value = NULL;
	} else {
		newmember->value = classmember->second->value->share();
	}
	this->operator[](classmember->first) = newmember;
	return;
}
