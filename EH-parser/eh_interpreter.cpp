/*
 * eh_interpreter.c
 * Jelle Zijlstra, December 2011
 *
 * Implements a Lex- and Yacc-based interpreter for the EH scripting language.
 */
#include "eh.h"
#include "eh_error.h"
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

static inline int count_nodes(const ehretval_p node);

/*
 * macros for interpreter behavior
 */
// take ints, return an int
#define EH_INT_CASE(token, operator) case token: \
	operand1 = eh_xtoint(eh_execute(node->opval->paras[0], context)); \
	operand2 = eh_xtoint(eh_execute(node->opval->paras[1], context)); \
	if(operand1->type() == int_e && operand2->type() == int_e) { \
		return ehretval_t::make((int) (operand1->intval operator operand2->intval)); \
	} else {\
		eh_error_types(#operator, operand1->type(), operand2->type(), eerror_e); \
	} \
	break;
// take ints or floats, return an int or float
#define EH_FLOATINT_CASE(token, operator) case token: \
	operand1 = eh_execute(node->opval->paras[0], context); \
	operand2 = eh_execute(node->opval->paras[1], context); \
	if(operand1->type() == float_e && operand2->type() == float_e) { \
		return ehretval_t::make((float) (operand1->floatval operator operand2->floatval)); \
	} else { \
		operand1 = eh_xtoint(operand1); \
		operand2 = eh_xtoint(operand2); \
		if(operand1->type() == int_e && operand2->type() == int_e) { \
			return ehretval_t::make((int) (operand1->intval operator operand2->intval)); \
		} else { \
			eh_error_types(#operator, operand1->type(), operand2->type(), eerror_e); \
		} \
	} \
	break;
// take ints or floats, return a bool
#define EH_INTBOOL_CASE(token, operator) case token: \
	operand1 = eh_execute(node->opval->paras[0], context); \
	operand2 = eh_execute(node->opval->paras[1], context); \
	if(operand1->type() == float_e && operand2->type() == float_e) { \
		return ehretval_t::make((bool) (operand1->floatval operator operand2->floatval)); \
	} else { \
		operand1 = eh_xtoint(operand1); \
		operand2 = eh_xtoint(operand2); \
		if(operand1->type() == int_e && operand2->type() == int_e) { \
			return ehretval_t::make((bool) (operand1->intval operator operand2->intval)); \
		} else { \
			eh_error_types(#operator, operand1->type(), operand2->type(), eerror_e); \
		} \
	} \
	break;
// take bools, return a bool
#define EH_BOOL_CASE(token, operator) case token: \
	operand1 = eh_xtobool(eh_execute(node->opval->paras[0], context)); \
	operand2 = eh_xtobool(eh_execute(node->opval->paras[1], context)); \
	return ehretval_t::make((bool) (operand1->boolval operator operand2->boolval)); \
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
EHI::EHI() : eval_parser(NULL), inloop(0), breaking(0), continuing(0), cmdtable(), is_strange_arrow(false), returning(false), global_object(NULL) {
	eh_init();
}
void EHI::eh_init(void) {
	global_object = new ehobj_t("AnonymousClass");
	
	for(int i = 0; libfuncs[i].code != NULL; i++) {
		ehmember_p func;
		func->value->type(func_e);
		func->value->funcval = new ehobj_t("Closure", global_object);
		ehfm_p f;
		f->type = lib_e;
		f->libfunc_pointer = libfuncs[i].code;
		func->value->funcval->function = f;
		// other fields are irrelevant
		global_object->insert(libfuncs[i].name, func);
	}
	for(int i = 0; libclasses[i].name != NULL; i++) {
		ehobj_t *newclass = new ehobj_t(libclasses[i].name);
		newclass->constructor = libclasses[i].info.constructor;
		ehlm_listentry_t *members = libclasses[i].info.members;
		// attributes for library methods
		attributes_t attributes = attributes_t::make(public_e, nonstatic_e, nonconst_e);
		for(int i = 0; members[i].name != NULL; i++) {
			ehmember_p func;
			func->attribute = attributes;
			func->value->type(func_e);
			func->value->funcval = new ehobj_t("Closure", newclass);
			ehfm_p f;
			f->type = libmethod_e;
			f->libmethod_pointer = members[i].func;
			func->value->funcval->function = f;
			newclass->insert(members[i].name, func);
		}
		ehmember_p member;
		member->attribute = attributes;
		member->value->set(newclass);
		global_object->insert(newclass->classname, member);
	}
	for(int i = 0; libcmds[i].name != NULL; i++) {
		insert_command(libcmds[i].name, libcmds[i].cmd);
	}
	for(int i = 0; libredirs[i][0] != NULL; i++) {
		redirect_command(libredirs[i][0], libredirs[i][1]);
	}
	// insert reference to global object
	attributes_t attributes = attributes_t::make(public_e, nonstatic_e, const_e);
	ehmember_p global;
	global->attribute = attributes;
	global->value->set(global_object);
	global->value->type(weak_object_e);
	global_object->insert("global", global);
	return;
}
void EHI::eh_exit(void) {
	//this->global_object->members.erase("global");
	if(eval_parser != NULL) {
		delete eval_parser;
	}
	delete global_object;
	return;
}
EHI::~EHI() {
	eh_exit();
}

/*
 * Main execution function
 */
ehretval_p EHI::eh_execute(ehretval_p node, const ehcontext_t context) {
	// variables used
	ehretval_p ret, operand1, operand2;
	bool b1, b2;

	// empty statements produce a null node
	if(node == NULL) {
		return ret;
	}
	try {
		if(node->type() == op_e) {
			//printf("Opcode %d: %d\n", node->opval->op, node->opval->nparas);
			switch(node->opval->op) {
				case T_LITERAL:
					if(node->opval->nparas == 0) {
						return NULL;
					} else {
						return node->opval->paras[0];
					}
			/*
			 * Unary operators
			 */
				case '@': // type casting
					ret = eh_cast(
						eh_execute(node->opval->paras[0], context)->typeval,
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
					return ehretval_t::make(!eh_xtobool(operand1));
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
						ehretval_p new_node = node->opval->paras[1];
						if(new_node->type() == op_e && new_node->opval->op == T_SEPARATOR && new_node->opval->nparas == 0) {
							return ret;
						} else {
							ret = eh_execute(new_node, context);
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
						return ehretval_t::make_typed(attributestr_e);
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
					return ehretval_t::make(eh_strictequals(
						eh_execute(node->opval->paras[0], context),
						eh_execute(node->opval->paras[1], context)
					));
				case T_SNE: // strict non-equality
					return ehretval_t::make(!eh_strictequals(
						eh_execute(node->opval->paras[0], context),
						eh_execute(node->opval->paras[1], context)
					));
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
						return ehretval_t::make(false);
					} else {
						operand2 = eh_execute(node->opval->paras[1], context);
						return ehretval_t::make(eh_xtobool(operand2));
					}
				case T_OR: // OR; use short-circuit operation
					operand1 = eh_execute(node->opval->paras[0], context);
					if(eh_xtobool(operand1)) {
						return ehretval_t::make(true);
					} else {
						operand2 = eh_execute(node->opval->paras[1], context);
						return ehretval_t::make(eh_xtobool(operand2));
					}
				case T_XOR:
					operand1 = eh_execute(node->opval->paras[0], context);
					operand2 = eh_execute(node->opval->paras[1], context);
					b1 = eh_xtobool(operand1);
					b2 = eh_xtobool(operand2);
					return ehretval_t::make((b1 && !b2) || (!b1 && b2));
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
					if(operand1->type() == null_e)
						break;
					operand2 = eh_xtoint(eh_execute(node->opval->paras[1], context));
					if(operand2->type() == null_e)
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
						switch(operand1->type()) {
							case int_e:
								operand1->intval--;
								break;
							default:
								eh_error_type("-- operator", operand1->type(), eerror_e);
								break;
						}
					}
					break;
				case T_PLUSPLUS:
					operand1 = eh_execute(node->opval->paras[0], context);
					if(operand1 == NULL) {
						eh_error("Cannot set with ++ operator", eerror_e);
					} else {
						switch(operand1->type()) {
							case int_e:
								operand1->intval++;
								break;
							default:
								eh_error_type("++ operator", operand1->type(), eerror_e);
								break;
						}
					}
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
ehretval_p EHI::eh_op_command(const char *name, ehretval_p node, ehcontext_t context) {
	ehretval_p value_r;
	// count for simple parameters
	int count = 0;
	// we're making an array of parameters
	eharray_t paras;
	// loop through the paras given
	for( ; node->opval->nparas != 0; node = node->opval->paras[1]) {
		ehretval_p node2 = node->opval->paras[0];
		if(node2->type() == op_e) {
			switch(node2->opval->op) {
				case T_SHORTPARA:
					// short paras: set each short-form option to the same thing
					if(node2->opval->nparas == 2) {
						// set to something else if specified
						value_r = eh_execute(node2->opval->paras[1], context);
					} else {
						// set to true by default
						value_r->set(true);
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
						paras.string_indices[index]->set(true);
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
	paras.string_indices["_ehphp"]->set(true);
	// get the command to execute
	const ehcmd_t libcmd = get_command(name);
	ehretval_p ret;
	if(libcmd != NULL) {
		ret = libcmd(&paras);
	} else {
		ret = execute_cmd(name, &paras);
	}
	// we're not returning anymore
	returning = false;
	return ret;
}
ehretval_p EHI::eh_op_for(opnode_t *op, ehcontext_t context) {
	ehretval_p ret = NULL;
	inloop++;
	breaking = 0;
	ehrange_t range;

	// get the count
	ehretval_p count_r = eh_execute(op->paras[0], context);
	if(count_r->type() == range_e) {
		range = *count_r->rangeval;
	} else {
		count_r = eh_xtoint(count_r);
		if(count_r->type() != int_e) {
			eh_error_type("count", count_r->type(), eerror_e);
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
		char *name = eh_execute(op->paras[1], context)->stringval;
		// this should perhaps create a new variable, or only overwrite variables in the current scope
		ehmember_p var = context->get_recursive(name, context, T_LVALUE_SET);
		// if we do T_LVALUE_SET, get_recursive never returns NULL
		// count variable always gets to be an int
		var->value->set((int) range.min);
		for( ; var->value->intval <= range.max; var->value->intval++) {
			ret = eh_execute(op->paras[2], context);
			LOOPCHECKS;
		}
	}
	inloop--;
	return ret;
}
ehretval_p EHI::eh_op_while(ehretval_p *paras, ehcontext_t context) {
	ehretval_p ret = NULL;
	inloop++;
	breaking = 0;
	while(eh_xtobool(eh_execute(paras[0], context))) {
		ret = eh_execute(paras[1], context);
		LOOPCHECKS;
	}
	inloop--;
	return ret;
}
ehretval_p EHI::eh_op_as(opnode_t *op, ehcontext_t context) {
	ehretval_p ret = NULL;

	// get the object to be looped through and check its type
	ehretval_p object = eh_execute(op->paras[0], context);
	if(object->type() != array_e && object->type() != object_e && object->type() != weak_object_e) {
		eh_error_type("for ... as operator", object->type(), enotice_e);
		return ret;
	}
	// increment loop count
	inloop++;
	// establish variables
	char *membername;
	ehmember_p membervar;
	char *indexname;
	ehmember_p indexvar = NULL;
	ehretval_p code;
	if(op->nparas == 3) {
		// no index
		membername = eh_execute(op->paras[1], context)->stringval;
		indexname = NULL;
		code = op->paras[2];
	} else {
		// with index
		indexname = eh_execute(op->paras[1], context)->stringval;
		membername = eh_execute(op->paras[2], context)->stringval;
		code = op->paras[3];
	}
	// create variables
	membervar = context->get_recursive(membername, context, T_LVALUE_SET);
	if(indexname != NULL) {
		indexvar = context->get_recursive(indexname, context, T_LVALUE_SET);
	}
	if(object->type() == object_e || object->type() == weak_object_e) {
		// object index is always a string
		if(indexname != NULL) {
			indexvar->value->type(string_e);
		}
		// check whether we're allowed to access private things
		const bool doprivate = ehcontext_compare(object->objectval, context);
		OBJECT_FOR_EACH(object->objectval, curr) {
			// ignore private
			if(!doprivate && curr->second->attribute.visibility == private_e) {
				continue;
			}
			membervar->value = curr->second->value;
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
			indexvar->value->type(int_e);
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
			indexvar->value->type(string_e);
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
ehretval_p EHI::eh_op_new(ehretval_p *paras, ehcontext_t context) {
	ehobj_t *classobj = this->get_class(eh_execute(paras[0], context), context);
	// get_class complains for us
	if(classobj == NULL) {
		return NULL;
	} else {
		return ehretval_t::make(this->object_instantiate(classobj));
	}
}
void EHI::eh_op_inherit(ehretval_p *paras, ehcontext_t context) {
	ehobj_t *classobj = this->get_class(eh_execute(paras[0], context), context);
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
		ehretval_p level_v = eh_xtoint(eh_execute(op->paras[0], context));
		if(level_v->type() != int_e) {
			return;
		} else {
			level = level_v->intval;
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
		ehretval_p level_v = eh_xtoint(eh_execute(op->paras[0], context));
		if(level_v->type() != int_e) {
			return;
		} else {
			level = level_v->intval;
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
ehretval_p EHI::eh_op_array(ehretval_p node, ehcontext_t context) {
	ehretval_p ret = ehretval_t::make(new eharray_t);
	// need to count array members first, because they are reversed in our node.
	// That's not necessary with functions (where the situation is analogous), because the reversals that happen when parsing the prototype argument list and parsing the argument list in a call cancel each other out.
	int count = 0;
	for(ehretval_p node2 = node; node2->opval->nparas != 0; node2 = node2->opval->paras[0]) {
		count++;
	}
	for(ehretval_p node2 = node; node2->opval->nparas != 0; node2 = node2->opval->paras[0]) {
		array_insert(ret->arrayval, node2->opval->paras[1], --count, context);
	}
	return ret;
}
ehretval_p EHI::eh_op_anonclass(ehretval_p node, ehcontext_t context) {
	ehretval_p ret = ehretval_t::make(new ehobj_t("AnonClass", context));
	// all members are public, non-static, non-const
	attributes_t attributes = attributes_t::make(public_e, nonstatic_e, nonconst_e);

	for( ; node->opval->nparas != 0; node = node->opval->paras[0]) {
		ehretval_p *myparas = node->opval->paras[1]->opval->paras;
		// nodes here will always have the name in para 0 and value in para 1
		ehretval_p namev = eh_execute(myparas[0], ret->objectval);
		if(namev->type() != string_e) {
			eh_error_type("Class member label", namev->type(), eerror_e);
			continue;
		}
		ehretval_p value = eh_execute(myparas[1], ret->objectval);
		ret->objectval->insert_retval(namev->stringval, attributes, value);
	}
	return ret;
}
ehretval_p EHI::eh_op_declareclosure(ehretval_p *paras, ehcontext_t context) {
	ehretval_p ret = ehretval_t::make_typed(func_e);
	ret->funcval = new ehobj_t;
	ret->funcval->parent = context;
	ret->funcval->classname = "Closure";

	ehfm_p f;
	f->type = user_e;
	f->code = paras[1];
	ret->funcval->function = f;

	// determine argument count
	f->argcount = count_nodes(paras[0]);
	// if there are no arguments, the arglist can be NULL
	if(f->argcount) {
		f->args = new eharg_t[f->argcount]();
	}
	// add arguments to arglist
	int i = 0;
	for(ehretval_p tmp = paras[0]; tmp->opval->nparas != 0; 
		tmp = tmp->opval->paras[0]) {
		f->args[i].name = eh_execute(tmp->opval->paras[1], context)->stringval;
		i++;
	}
	return ret;
}
ehretval_p EHI::eh_op_declareclass(opnode_t *op, ehcontext_t context) {
	// process parameters
	const char *name;
	ehretval_p code;
	if(op->nparas == 2) {
		name = eh_execute(op->paras[0], context)->stringval;
		code = op->paras[1];
	} else {
		name = "AnonymousClass";
		code = op->paras[0];
	}

	ehobj_t *classobj = new ehobj_t();
	classobj->parent = context;
	classobj->classname = name;

	// insert "this" pointer
	attributes_t thisattributes = attributes_t::make(private_e, nonstatic_e, const_e);
	ehretval_p thisvalue;
	thisvalue->set(classobj);
	thisvalue->type(weak_object_e);
	classobj->insert_retval("this", thisattributes, thisvalue);

	eh_execute(code, classobj);
	
	// create the ehretval_t
	ehretval_p ret = ehretval_t::make(classobj);
	if(op->nparas == 2) {
		// insert variable
		ehmember_p member;
		member->value = ret;
		context->insert(name, member);
	}
	return ret;
}
void EHI::eh_op_classmember(opnode_t *op, ehcontext_t context) {
	// rely on standard layout of the paras
	ehretval_p attribute_v = eh_execute(op->paras[0], context);
	attributes_t attribute = attribute_v->attributestrval;
	char *name = eh_execute(op->paras[1], context)->stringval;

	// decide what we got
	ehretval_p value;
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
ehretval_p EHI::eh_op_switch(ehretval_p *paras, ehcontext_t context) {
	ehretval_p ret;
	// because we use continue, we'll pretend this is a loop
	inloop++;

	// switch variable
	ehretval_p switchvar = eh_execute(paras[0], context);
	for(ehretval_p node = paras[1]; node->opval->nparas != 0; node = node->opval->paras[1]) {
		opnode_t *op = node->opval->paras[0]->opval;
		// execute default
		if(op->nparas == 1) {
			ret = eh_execute(op->paras[0], context);
		} else {
			ehretval_p casevar = eh_execute(op->paras[0], context);
			ehretval_p decider;
			// try to call function
			if(casevar->type() == func_e) {
				decider = call_function_args(casevar->funcval, 1, &switchvar, context);
				if(decider->type() != bool_e) {
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
		} else {
			return ret;
		}
	}
	return NULL;
}
ehretval_p EHI::eh_op_given(ehretval_p *paras, ehcontext_t context) {
	// switch variable
	ehretval_p switchvar = eh_execute(paras[0], context);
	for(ehretval_p node = paras[1]; node->opval->nparas != 0; node = node->opval->paras[1]) {
		const opnode_t *op = node->opval->paras[0]->opval;
		// execute default
		if(op->nparas == 1) {
			return eh_execute(op->paras[0], context);
		}
		ehretval_p casevar = eh_execute(op->paras[0], context);
		ehretval_p decider;
		if(casevar->type() == func_e) {
			decider = call_function_args(
				casevar->funcval, 1, &switchvar, context
			);
			if(decider->type() != bool_e) {
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
ehretval_p EHI::eh_op_colon(ehretval_p *paras, ehcontext_t context) {
	ehretval_p function = eh_execute(paras[0], context);
	// operand1 will be either a string (indicating a normal function call) or a 
	// func_e (indicating a method or closure call)
	switch(function->type()) {
		case string_e:
		{
			ehmember_p func = context->get_recursive(
				function->stringval, context, T_LVALUE_GET
			);
			if(func == NULL) {
				eh_error_unknown("function", function->stringval, eerror_e);
				return NULL;
			}
			if(func->value->type() != func_e) {
				eh_error_type("function call", func->value->type(), eerror_e);
				return NULL;
			}
			return call_function(func->value->funcval, paras[1], context);
		}
		case func_e:
			return call_function(function->funcval, paras[1], context);
		case null_e:
			// ignore null functions to prevent duplicate warnings
			return NULL;
		default:
			eh_error_type("function call", function->type(), eerror_e);
			return NULL;
	}
	return NULL;
}
ehretval_p &EHI::eh_op_lvalue(opnode_t *op, ehcontext_t context) {
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
	ehretval_p basevar = eh_execute(op->paras[0], context);
	switch(op->nparas) {
		case 1:
		{
			ehmember_p var = context->get_recursive(basevar->stringval, context, op->op);
			// dereference variable
			if(var != NULL) {
				return var->value;
			}
			/*
			 * If there is no variable of this name, and it is a
			 * simple access, we throw.
			 */
			break;
		}
		case 3:
			switch(eh_execute(op->paras[1], context)->accessorval) {
				case arrow_e:
					return object_access(basevar, op->paras[2], context, op->op);
				case doublecolon_e:
					return colon_access(basevar, op->paras[2], context, op->op);
			}
			break;
	}
	throw 0;
}
ehretval_p EHI::eh_op_dollar(ehretval_p node, ehcontext_t context) {
	ehretval_p ret = eh_execute(node, context);
	if(ret == NULL) {
		return NULL;
	}
	
	ehretval_p varname = eh_xtostring(ret);
	if(varname == NULL) {
		return NULL;
	}
	
	ehmember_p var = context->get_recursive(
		varname->stringval, context, T_LVALUE_GET
	);
	if(var == NULL) {
		return NULL;
	} else {
		return var->value;
	}
}
void EHI::eh_op_set(ehretval_p *paras, ehcontext_t context) {
	try {
		this->is_strange_arrow = false;
		ehretval_p &lvalue = eh_op_lvalue(paras[0]->opval, context);
		bool is_strange = this->is_strange_arrow;
		this->is_strange_arrow = false;
		ehretval_p rvalue = eh_execute(paras[1], context);
		if(is_strange) {
			// lvalue is a pointer to the variable modified, rvalue is the value set to, index is the index
			ehretval_p index = eh_execute(paras[0]->opval->paras[2], context);
			switch(lvalue->type()) {
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
					eh_error_type("arrow access", lvalue->type(), eerror_e);
					break;
			}
		} else {
			lvalue = rvalue;
		}
	} catch(int) {
		// do nothing
	}
}
ehretval_p EHI::eh_op_accessor(ehretval_p *paras, ehcontext_t context) {
	// this only gets executed for array-type int and string access
	ehretval_p ret;
	ehretval_p basevar = eh_execute(paras[0], context);
	switch(eh_execute(paras[1], context)->accessorval) {
		case arrow_e:
		{
			ehretval_p index = eh_execute(paras[2], context);
			// "array" access
			switch(basevar->type()) {
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
				case weak_object_e:
				case object_e:
					if(index->type() != string_e) {
						eh_error_type("access to object", index->type(), eerror_e);
					} else {
						ehmember_p member = basevar->objectval->get(index->stringval, context, T_LVALUE_GET);
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
					eh_error_type("arrow access", basevar->type(), eerror_e);
					break;
			}
			break;
		}
		case doublecolon_e:
			try {
				ret = colon_access(basevar, paras[2], context, T_LVALUE_GET);
			} catch(int) {
			}
			break;
		default:
			eh_error("Unsupported accessor", efatal_e);
			break;
	}
	return ret;
}
/*
 * Functions
 */
ehretval_p EHI::call_function(ehobj_t *obj, ehretval_p args, ehcontext_t context) {
	// this is a wrapper for call_function_args; it parses the arguments and
	// puts them in an array
	int nargs = count_nodes(args);
	ehretval_p *new_args = new ehretval_p[nargs]();
	
	for(int i = 0; args->opval->nparas != 0; args = args->opval->paras[0], i++) {
		new_args[i] = eh_execute(args->opval->paras[1], context);
	}
	ehretval_p ret = this->call_function_args(obj, nargs, new_args, context);
	delete[] new_args;

	return ret;
}
ehretval_p EHI::call_function_args(ehobj_t *obj, const int nargs, ehretval_p args[], ehcontext_t context) {
	ehretval_p ret;
	
	ehfm_p f = obj->function;
	if(f == NULL) {
		eh_error("Invalid object for function call", eerror_e);
		return NULL;
	}

	if(f->type == lib_e) {
		return f->libfunc_pointer(nargs, args, context, this);
	} else if(f->type == libmethod_e) {
		return f->libmethod_pointer(obj->parent->selfptr, nargs, args, context, this);
	}
	// check parameter count
	if(nargs != f->argcount) {
		eh_error_argcount(f->argcount, nargs);
		return NULL;
	}
	ehobj_t *newcontext = object_instantiate(obj);
	
	// set parameters as necessary
	for(int i = 0; i < nargs; i++) {
		ehmember_p var;
		var->value = args[i];
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
	if(obj->constructor != NULL) {
		// insert selfptr
		ret->selfptr = obj->constructor();
	}
	
	ehretval_p constructor = NULL;
	for(int i = 0; i < VARTABLE_S; i++) {
		OBJECT_FOR_EACH(obj, m) {
			ret->copy_member(m, false);
			if(m->first.compare("constructor") == 0) {
				constructor = m->second->value;
			}
		}
	}
	if(constructor != NULL) {
		if(constructor->type() != func_e) {
			eh_error_type("constructor", constructor->type(), enotice_e);
		} else {
			call_function_args(constructor->funcval, 0, NULL, obj->parent);
		}
	}
	return ret;
}
ehretval_p &EHI::object_access(ehretval_p operand1, ehretval_p index, ehcontext_t context, int token) {
	ehmember_p member;

	ehmember_p var = context->get_recursive(operand1->stringval, context, T_LVALUE_GET);
	if(var == NULL) {
		eh_error("cannot access member of nonexistent variable", eerror_e);
		throw 0;
	}
	ehretval_p label;

	switch(var->value->type()) {
		case array_e:
			label = eh_execute(index, context);
			if(var->value->arrayval->has(label) or (token == T_LVALUE_SET)) {
				return var->value->arrayval->operator[](label);
			} else {
				throw 0;
			}
		case weak_object_e:
		case object_e:
			label = eh_execute(index, context);
			if(label->type() != string_e) {
				eh_error_type("object member label", label->type(), eerror_e);
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
			this->is_strange_arrow = true;
			return var->value;
	}
	throw 0;
}
ehretval_p &EHI::colon_access(ehretval_p operand1, ehretval_p index, ehcontext_t context, int token) {
	ehretval_p label = eh_execute(index, context);
	if(label->type() != string_e) {
		eh_error_type("object member label", label->type(), eerror_e);
		throw 0;
	}

	ehobj_t *classobj = this->get_class(operand1, context);
	if(classobj == NULL) {
		throw 0;
	}
	ehmember_p member = classobj->get(label->stringval, context, token);
	if(member == NULL) {
		if(token == T_LVALUE_GET) {
			eh_error_unknown("object member", label->stringval, eerror_e);		
		}
		throw 0;
	} else {
		return member->value;
	}
}
ehobj_t *EHI::get_class(ehretval_p classname, ehcontext_t context) {
	ehobj_t *classobj;
	switch(classname->type()) {
		case string_e:
		{
			ehmember_p member = context->get_recursive(classname->stringval, context, T_LVALUE_GET);
			if(member == NULL) {
				eh_error_unknown("class", classname->stringval, eerror_e);
				return NULL;
			}
			if(member->value->type() != object_e && member->value->type() != weak_object_e) {
				eh_error_type("class", member->value->type(), eerror_e);
				return NULL;
			}
			classobj = member->value->objectval;
			break;
		}
		case weak_object_e:
		case object_e:
			classobj = classname->objectval;
			break;
		default:
			eh_error_type("class name", classname->type(), eerror_e);
			return NULL;
	}
	return classobj;
}

/*
 * Arrays
 */
void EHI::array_insert(eharray_t *array, ehretval_p in, int place, ehcontext_t context) {
	/*
	 * We'll assume we're always getting a correct ehretval_p , referring to a
	 * T_ARRAYMEMBER token. If there is 1 parameter, that means it's a
	 * non-labeled array member, which we'll give an integer array index; if
	 * there are 2, we'll either use the integer array index or a hash of the
	 * string index.
	 */
	if(in->opval->nparas == 1) {
		// if there is no explicit key, simply use the place argument
		array->int_indices[place] = eh_execute(in->opval->paras[0], context);
	} else {
		const ehretval_p label = eh_execute(in->opval->paras[0], context);
		ehretval_p var = eh_execute(in->opval->paras[1], context);
		switch(label->type()) {
			case int_e:
				array->int_indices[label->intval] = var;
				break;
			case string_e:
				array->string_indices[label->stringval] = var;
				break;
			default:
				eh_error_type("array member label", label->type(), enotice_e);
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
	ehmember_p argc_v;
	// argc - 1, because argv[0] is ehi itself
	argc_v->value->set((int) argc - 1);
	global_object->insert("argc", argc_v);

	// insert argv
	ehmember_p argv_v;
	argv_v->value->set(new eharray_t);

	// all members of argv are strings
	for(int i = 1; i < argc; i++) {
		argv_v->value->arrayval->int_indices[i - 1]->set(argv[i]);
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
ehretval_p eh_count(const ehretval_p in) {
	ehretval_p ret;
	ret->type(int_e);
	switch(in->type()) {
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
			eh_error_type("count operator", in->type(), eerror_e);
			ret->type(null_e);
			break;
	}
	return ret;
}
ehretval_p eh_op_tilde(ehretval_p in) {
	// no const argument because it's modified below
	switch(in->type()) {
		// bitwise negation of a bool is just normal negation
		case bool_e:
			return ehretval_t::make(!in->boolval);
		// else try to cast to int
		default:
			in = eh_xtoint(in);
			if(in->type() != int_e) {
				eh_error_type("bitwise negation", in->type(), eerror_e);
				return NULL;
			}
			// fall through to int case
		case int_e:
			return ehretval_t::make(~in->intval);
	}
	return NULL;
}
ehretval_p eh_op_uminus(ehretval_p in) {
	switch(in->type()) {
		// negation
		case bool_e:
			return ehretval_t::make(!in->boolval);
		case float_e:
			return ehretval_t::make(-in->floatval);
		default:
			in = eh_xtoint(in);
			if(in->type() != int_e) {
				eh_error_type("negation", in->type(), eerror_e);
				return NULL;
			}
			// fall through to int case
		case int_e:
			return ehretval_t::make(-in->intval);
	}
	return NULL;
}
ehretval_p eh_op_dot(ehretval_p operand1, ehretval_p operand2) {
	operand1 = eh_xtostring(operand1);
	operand2 = eh_xtostring(operand2);
	if(operand1->type() == string_e && operand2->type() == string_e) {
		// concatenate them
		size_t len1 = strlen(operand1->stringval);
		size_t len2 = strlen(operand2->stringval);
		char *out = new char[len1 + len2 + 1];
		strcpy(out, operand1->stringval);
		strcpy(out + len1, operand2->stringval);
		return ehretval_t::make(out);
	} else {
		return NULL;
	}
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
ehretval_p eh_cast(const type_enum type, ehretval_p in) {
	switch(type) {
// macro for the common case
#define EH_CAST_CASE(vtype) case vtype ## _e: \
	return ehretval_t::make(eh_xto ## vtype (in));
		EH_CAST_CASE(int)
		EH_CAST_CASE(string)
		EH_CAST_CASE(float)
		EH_CAST_CASE(range)
		EH_CAST_CASE(array)
#undef EH_CAST_CASE
		case bool_e:
			return ehretval_t::make(eh_xtobool(in));
		default:
			eh_error_type("typecast", type, eerror_e);
			break;
	}
	return NULL;
}

#define CASTERROR(totype) do { \
	eh_error_type("typecast to " #totype, in->type(), enotice_e); \
	return NULL; \
} while(0)
#define CASTERROR_KNOWN(totype, vtype) do { \
	eh_error_type("typecast to " #totype, vtype ## _e, enotice_e); \
	return NULL; \
} while(0)

/* Casts between specific pairs of types */
ehretval_p eh_stringtoint(const char *const in) {
	char *endptr;
	ehretval_p ret = ehretval_t::make((int) strtol(in, &endptr, 0));
	// If in == endptr, strtol read no digits and there was no conversion.
	if(in == endptr) {
		CASTERROR_KNOWN(int, string);
		return NULL;
	}
	return ret;
}
ehretval_p eh_stringtofloat(const char *const in) {
	char *endptr;
	ehretval_p ret = ehretval_t::make(strtof(in, &endptr));
	// If in == endptr, strtof read no digits and there was no conversion.
	if(in == endptr) {
		CASTERROR_KNOWN(float, string);
		return NULL;
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
ehretval_p eh_rangetoarray(const ehrange_t *const range) {
	ehretval_p ret = ehretval_t::make(new eharray_t);
	ret->arrayval->int_indices[0] = ehretval_t::make(range->min);
	ret->arrayval->int_indices[1] = ehretval_t::make(range->max);
	return ret;
}
ehretval_p eh_stringtorange(const char *const in) {
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
ehretval_p eh_xtoint(ehretval_p in) {
	switch(in->type()) {
		case int_e:
			return in;
		case string_e:
			return ehretval_t::make(eh_stringtoint(in->stringval));
		case bool_e:
			if(in->boolval) {
				return ehretval_t::make(1);
			} else {
				return ehretval_t::make(0);
			}
		case null_e:
			return ehretval_t::make(0);
		case float_e:
			return ehretval_t::make((int) in->floatval);
		default:
			CASTERROR(int);
	}
	return NULL;
}
ehretval_p eh_xtostring(ehretval_p in) {
	switch(in->type()) {
		case string_e:
			return in;
		case int_e:
			return ehretval_t::make(eh_inttostring(in->intval));
		case null_e: {
			// empty string
			char *out = new char[1];
			out[0] = '\0';
			return ehretval_t::make(out);
		}
		case bool_e: {
			char *out;
			if(in->boolval) {
				out = new char[5];
				strcpy(out, "true");
			} else {
				out = new char[6];
				strcpy(out, "false");
			}
			return ehretval_t::make(out);
		}
		case float_e:
			return ehretval_t::make(eh_floattostring(in->floatval));
		case range_e:
			return ehretval_t::make(eh_rangetostring(in->rangeval));
		case array_e: // Should implode the array
		case object_e: // Should call __toString-type method
		case func_e: // Can't think of anything useful
		default:
			CASTERROR(string);
	}
	return NULL;
}
bool eh_xtobool(ehretval_p in) {
	// convert an arbitrary variable to a bool
	switch(in->type()) {
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
		case weak_object_e:
		case func_e:
			// objects and functions are true if they exist
			return true;
		default:
			// other types are always false
			return false;
	}
	return false;
}
ehretval_p eh_xtofloat(ehretval_p in) {
	switch(in->type()) {
		case float_e:
			return in;
		case int_e:
			return ehretval_t::make((float) in->intval);
		case string_e:
			return eh_stringtofloat(in->stringval);
		case bool_e:
			if(in->boolval) {
				return ehretval_t::make(1.0);
			} else {
				return ehretval_t::make(0.0);
			}
		case null_e:
			return ehretval_t::make(0.0);
		default:
			CASTERROR(float);
	}
	return NULL;
}
ehretval_p eh_xtorange(ehretval_p in) {
	ehretval_p ret = NULL;
	switch(in->type()) {
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
ehretval_p eh_xtoarray(ehretval_p in) {
	switch(in->type()) {
		case array_e:
			return in;
		case range_e:
			return eh_rangetoarray(in->rangeval);
		case int_e:
		case bool_e:
		case string_e:
		case func_e:
		case null_e:
		case object_e:
		case weak_object_e: {
			ehretval_p ret = ehretval_t::make(new eharray_t);
			// create an array with just this variable in it
			ret->arrayval->int_indices[0] = in;
			return ret;
		}
		default:
			CASTERROR(array);
	}
	return NULL;
}
static inline bool eh_floatequals(float infloat, ehretval_p operand2) {
	ehretval_p operand = eh_xtoint(operand2);
	// checks whether a float equals an int. C handles this correctly.
	if(operand->type() != int_e) {
		return false;
	}
	return (infloat == operand->intval);
}
ehretval_p eh_looseequals(ehretval_p operand1, ehretval_p operand2) {
	// first try strict comparison
	if(eh_strictequals(operand1, operand2)) {
		return ehretval_t::make(true);
	} else if(operand1->type() == float_e) {
		return ehretval_t::make(eh_floatequals(operand1->floatval, operand2));
	} else if(operand2->type() == float_e) {
		return ehretval_t::make(eh_floatequals(operand2->floatval, operand1));
	} else {
		operand1 = eh_xtoint(operand1);
		operand2 = eh_xtoint(operand2);
		if(operand1->type() == int_e && operand2->type() == int_e) {
			return ehretval_t::make(operand1->intval == operand2->intval);
		} else {
			return NULL;
		}
	}
}
bool eh_strictequals(ehretval_p operand1, ehretval_p operand2) {
	if(operand1->type() != operand2->type()) {
		// strict comparison between different types always returns false
		return false;
	}
	switch(operand1->type()) {
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
ehretval_p int_arrow_get(ehretval_p operand1, ehretval_p operand2) {
	// "array" access to integer returns the nth bit of the integer; for example 
	// (assuming sizeof(int) == 32), (2 -> 30) == 1, (2 -> 31) == 0
	if(operand2->type() != int_e) {
		eh_error_type("bitwise access to integer", operand2->type(), enotice_e);
		return NULL;
	}
	if(operand2->intval >= (int) sizeof(int) * 8) {
		eh_error_int("Identifier too large for bitwise access to integer", 	
			operand2->intval, enotice_e);
		return NULL;
	}
	// get mask
	int mask = 1 << (sizeof(int) * 8 - 1);
	mask >>= operand2->intval;
	// apply mask
	return ehretval_t::make((int) (operand1->intval & mask) >> (sizeof(int) * 8 - 1 - mask));
}
ehretval_p string_arrow_get(ehretval_p operand1, ehretval_p operand2) {
	// "array" access to string returns integer representing nth character.
	// In the future, perhaps replace this with a char datatype or with a 
	// "shortstring" datatype representing strings up to 3 or even 4 characters 
	// long
	if(operand2->type() != int_e) {
		eh_error_type("character access to string", operand2->type(), enotice_e);
		return NULL;
	}
	int count = strlen(operand1->stringval);
	if(operand2->intval >= count) {
		eh_error_int("Identifier too large for character access to string", 
			operand2->intval, enotice_e);
		return NULL;
	}
	// get the nth character
	return ehretval_t::make((int) operand1->stringval[operand2->intval]);
}
ehretval_p range_arrow_get(ehretval_p range, ehretval_p accessor) {
	if(accessor->type() != int_e) {
		eh_error_type("arrow access to range", accessor->type(), enotice_e);
		return NULL;
	}
	switch(accessor->intval) {
		case 0:
			return ehretval_t::make(range->rangeval->min);
		case 1:
			return ehretval_t::make(range->rangeval->max);
		default:
			eh_error_int("invalid range accessor", accessor->intval, enotice_e);
			break;
	}
	return NULL;
}
void int_arrow_set(ehretval_p input, ehretval_p index, ehretval_p rvalue) {
	if(index->type() != int_e) {
		eh_error_type("bitwise access to integer", index->type(), enotice_e);
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
void string_arrow_set(ehretval_p input, ehretval_p index, ehretval_p rvalue) {
	if(rvalue->type() != int_e) {
		eh_error_type("character access to string", rvalue->type(), enotice_e);
		return;
	}
	if(index->type() != int_e) {
		eh_error_type("setting a character in a string", index->type(), enotice_e);
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
void range_arrow_set(ehretval_p input, ehretval_p index, ehretval_p rvalue) {
	if(rvalue->type() != int_e) {
		eh_error_type("arrow access to range", rvalue->type(), enotice_e);
	} else if(index->type() != int_e) {
		eh_error_type("arrow access to range", index->type(), enotice_e);
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
ehretval_p eh_make_range(const int min, const int max) {
	return ehretval_t::make(new ehrange_t(min, max));
}
static inline int count_nodes(const ehretval_p node) {
	// count a list like an argument list. Assumes correct layout.
	int i = 0;
	for(ehretval_p tmp = node; 
		tmp->opval->nparas != 0; 
		tmp = tmp->opval->paras[0], i++
	);
	return i;

}

/*
 * ehretval_t
 */
void ehretval_t::print() {
	switch(this->type()) {
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
			eh_error_type("echo operator", this->type(), enotice_e);
			break;
	}
	return;
}

/*
 * eharray_t
 */
ehretval_p &eharray_t::operator[](ehretval_p index) {
	switch(index->type()) {
		case int_e:
			return int_indices[index->intval];
		case string_e:
			return string_indices[index->stringval];
		default:
			eh_error_type("array index", index->type(), enotice_e);
			throw 0;
	}
}
void eharray_t::insert_retval(ehretval_p index, ehretval_p value) {
	// Inserts a member into an array. 
	switch(index->type()) {
		case int_e:
			this->int_indices[index->intval] = value;
			break;
		case string_e:
			this->string_indices[index->stringval] = value;
			break;
		default:
			eh_error_type("array index", index->type(), enotice_e);
			break;
	}
}
/*
 * ehobj_t
 */
ehmember_p ehobj_t::insert_retval(const char *name, attributes_t attribute, ehretval_p value) {
	if(this->has(name)) {
		eh_error("object member already set", enotice_e);
		return NULL;
	}
	// insert a member into a class
	ehmember_p member;
	member->attribute = attribute;
	member->value = value;

	// insert into object
	members[name] = member;
	return member;
}
ehmember_p ehobj_t::get(const char *name, const ehcontext_t context, int token) {
	if(this->has(name)) {
		ehmember_p out = this->members[name];
		if(out->attribute.visibility == private_e && !ehcontext_compare(this, context)) {
			return NULL;
		} else if(token == T_LVALUE_SET && out->attribute.isconst == const_e) {
			eh_error("Attempt to write to constant variable", eerror_e);
			return NULL;
		} else {
			return out;
		}
	} else if(token == T_LVALUE_SET) {
		ehmember_p member;
		this->insert(name, member);
		return this->members[name];
	} else {
		return NULL;
	}
}
ehmember_p ehobj_t::get_recursive(const char *name, ehcontext_t context, int token) {
	ehmember_p currvar = this->get_recursive_helper(name, context);
	if(token == T_LVALUE_SET) {
		if(currvar == NULL) {
			if(!this->has(name)) {
				ehmember_p newvar;
				this->insert(name, newvar);
				return this->members[name];
			} else {
				throw 0;
			}
		} else if(currvar->attribute.isconst == const_e) {
			eh_error("Attempt to write to constant variable", eerror_e);
			throw 0;
		}
	}
	// without this weird-looking code, it may create a useless ehmember_t
	if(currvar == NULL) {
		return NULL;
	} else {
		return currvar;
	}
}
ehmember_p ehobj_t::get_recursive_helper(const char *name, const ehcontext_t context) {
	if(this->has(name)) {
		return this->members[name];
	}
	if(this->real_parent == NULL) {
		if(this->parent != NULL) {
			return this->parent->get_recursive_helper(name, context);
		} else {
			return NULL;
		}
	} else {
		if(this->parent != NULL && this->parent->has(name)) {
			return this->parent->members[name];
		} else {
			return this->real_parent->get_recursive_helper(name, context);
		}
	}
}
void ehobj_t::copy_member(obj_iterator &classmember, bool set_real_parent) {
	ehmember_p newmember;
	if(classmember->first.compare("this") == 0) {
		// handle $this pointer
		newmember->attribute = classmember->second->attribute;
		newmember->value->set(this);
		newmember->value->type(weak_object_e);
	} else if(classmember->second->isstatic() || (classmember->second->isconst() && classmember->second->value->type() != func_e)) {
		// we can safely share static members, as well as const members that are not functions
		newmember = classmember->second;
	} else {
		newmember->attribute = classmember->second->attribute;
		if(classmember->second->value->type() == func_e) {
			newmember->value->type(func_e);
			ehobj_t *oldobj = classmember->second->value->funcval;
			ehobj_t *f = new ehobj_t(oldobj->classname, this);
			newmember->value->funcval = f;
			if(set_real_parent && oldobj->real_parent == NULL) {
				f->real_parent = oldobj->parent->parent;
			} else {
				f->real_parent = oldobj->real_parent;
			}
			f->function = oldobj->function;
			f->members = oldobj->members;
		} else {
			newmember->value = classmember->second->value;
		}
	}
	this->members[classmember->first] = newmember;
}
