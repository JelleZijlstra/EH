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
	ehlibmethod_t code;
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
	LIBFUNCENTRY(throw)
	{NULL, NULL}
};

typedef struct ehlc_listentry_t {
	const char *name;
	ehlm_listentry_t *members;
	int type_id;
} ehlc_listentry_t;
#define LIBCLASSENTRY(c, is_core) { #c, ehlc_l_ ## c, is_core},
ehlc_listentry_t libclasses[] = {
	LIBCLASSENTRY(Object, base_object_e)
	LIBCLASSENTRY(CountClass, -1)
	LIBCLASSENTRY(File, -1)
	LIBCLASSENTRY(Integer, int_e)
	LIBCLASSENTRY(String, string_e)
	LIBCLASSENTRY(Array, array_e)
	LIBCLASSENTRY(Float, float_e)
	LIBCLASSENTRY(Bool, bool_e)
	LIBCLASSENTRY(Null, null_e)
	LIBCLASSENTRY(Range, range_e)
	LIBCLASSENTRY(Hash, hash_e)
	{NULL, NULL, 0}
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
	operand1 = this->to_int(eh_execute(node->get_opval()->paras[0], context), context); \
	operand2 = this->to_int(eh_execute(node->get_opval()->paras[1], context), context); \
	if(operand1->type() == int_e && operand2->type() == int_e) { \
		return ehretval_t::make_int((operand1->get_intval() operator operand2->get_intval())); \
	} else {\
		eh_error_types(#operator, operand1->type(), operand2->type(), eerror_e); \
	} \
	break;
// take ints or floats, return a bool
#define EH_INTBOOL_CASE(token, operator) case token: \
	operand1 = eh_execute(node->get_opval()->paras[0], context); \
	operand2 = eh_execute(node->get_opval()->paras[1], context); \
	if(operand1->type() == float_e && operand2->type() == float_e) { \
		return ehretval_t::make_bool(operand1->get_floatval() operator operand2->get_floatval()); \
	} else { \
		operand1 = this->to_int(operand1, context); \
		operand2 = this->to_int(operand2, context); \
		if(operand1->type() == int_e && operand2->type() == int_e) { \
			return ehretval_t::make_bool(operand1->get_intval() operator operand2->get_intval()); \
		} else { \
			eh_error_types(#operator, operand1->type(), operand2->type(), eerror_e); \
		} \
	} \
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
EHI::EHI() : eval_parser(NULL), inloop(0), breaking(0), continuing(0), cmdtable(), is_strange_arrow(false), buffer(NULL), gc(), returning(false), repo(), global_object() {
	eh_init();
}
void EHI::eh_init(void) {
	global_object = this->make_object(new ehobj_t());
	
	for(int i = 0; libfuncs[i].code != NULL; i++) {
		ehmember_p func;
		ehobj_t *obj = new ehobj_t();
		obj->type_id = func_e;
		obj->parent = global_object;
		ehfunc_t *f = new ehfunc_t(lib_e);
		f->libmethod_pointer = libfuncs[i].code;
		obj->object_data = ehretval_t::make_func(f);
		func->value = this->make_object(obj);
		global_object->get_objectval()->insert(libfuncs[i].name, func);
	}
	ehobj_t *base_object;
	for(int i = 0; libclasses[i].name != NULL; i++) {
		ehobj_t *newclass = new ehobj_t();
		ehretval_p new_value = this->make_object(newclass);
		// register class
		int type_id;
		if(libclasses[i].type_id == -1) {
			type_id = this->repo.register_class(libclasses[i].name, new_value);
		} else {
			type_id = libclasses[i].type_id;
			this->repo.register_known_class(type_id, libclasses[i].name, new_value);
		}
		newclass->type_id = type_id;

		// inherit from Object. This relies on Object always being the first library class loaded
		if(libclasses[i].type_id == base_object_e) {
			base_object = newclass;
		} else {
			OBJECT_FOR_EACH(base_object, member) {
				newclass->copy_member(member, false, new_value, this);
			}
		}
		ehlm_listentry_t *members = libclasses[i].members;
		// attributes for library methods
		attributes_t attributes = attributes_t::make(public_e, nonstatic_e, nonconst_e);
		for(int i = 0; members[i].name != NULL; i++) {
			ehmember_p func;
			func->attribute = attributes;
			ehobj_t *function_object = new ehobj_t();
			func->value = this->make_object(function_object);
			function_object->parent = new_value;
			function_object->type_id = func_e;
			ehfunc_t *f = new ehfunc_t(lib_e);
			f->libmethod_pointer = members[i].func;
			function_object->object_data = ehretval_t::make_func(f);
			newclass->insert(members[i].name, func);
		}
		ehmember_p member;
		// library classes themselves are constant; otherwise the engine might blow up
		attributes.isconst = const_e;
		member->attribute = attributes;
		member->value = new_value;
		global_object->get_objectval()->insert(libclasses[i].name, member);
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
	global->value = global_object;
	global_object->get_objectval()->insert("global", global);
}
void EHI::eh_exit(void) {
	if(this->eval_parser != NULL) {
		delete this->eval_parser;
	}
	if(this->buffer != NULL) {
		delete[] this->buffer;
	}
	this->global_object->get_objectval()->members.erase("global");
	this->gc.do_collect(this->global_object);
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
			//printf("Opcode %d: %d\n", node->opval->op, node->get_opval()->nparas);
			switch(node->get_opval()->op) {
				case T_LITERAL:
					if(node->get_opval()->nparas == 0) {
						return NULL;
					} else {
						return node->get_opval()->paras[0];
					}
			/*
			 * Unary operators
			 */
				case '@': // type casting
					ret = eh_cast(
						eh_execute(node->get_opval()->paras[0], context)->get_typeval(),
						eh_execute(node->get_opval()->paras[1], context),
						context
					);
					break;
				case '~': // bitwise negation
				  return perform_op("operator_tilde", "operator~", 0, node->get_opval()->paras, context);
				case T_NEGATIVE: // sign change
				  return perform_op("operator_uminus", "operator-", 0, node->get_opval()->paras, context);
				case '!': // Boolean not
				  return perform_op("operator_bang", "operator!", 0, node->get_opval()->paras, context);
			/*
			 * Control flow
			 */
				case T_IF:
					operand1 = eh_execute(node->get_opval()->paras[0], context);
					if(this->to_bool(operand1, context)->get_boolval()) {
						ret = eh_execute(node->get_opval()->paras[1], context);
					} else if(node->get_opval()->nparas == 3) {
						ret = eh_execute(node->get_opval()->paras[2], context);
					}
					break;
				case T_WHILE:
					ret = eh_op_while(node->get_opval()->paras, context);
					break;
				case T_FOR:
					ret = eh_op_for(node->get_opval(), context);
					break;
				case T_AS:
					ret = eh_op_as(node->get_opval(), context);
					break;
				case T_SWITCH: // switch statements
					ret = eh_op_switch(node->get_opval()->paras, context);
					// incremented in the eh_op_switch function
					inloop--;
					break;
				case T_GIVEN: // inline switch statements
					ret = eh_op_given(node->get_opval()->paras, context);
					break;
				case T_TRY:
				  ret = eh_op_try(node->get_opval()->paras, context);
				  break;
			/*
			 * Miscellaneous
			 */
				case T_SEPARATOR:
					// if we're in an empty list
					if(node->get_opval()->nparas == 0) {
						return ret;
					}
					// else execute both commands
					ret = eh_execute(node->get_opval()->paras[0], context);
					if(returning || breaking || continuing) {
						return ret;
					} else {
						// check for empty statement; this means that the last
						// actual statement in a function is returned
						ehretval_p new_node = node->get_opval()->paras[1];
						if(new_node->type() == op_e && new_node->get_opval()->op == T_SEPARATOR && new_node->get_opval()->nparas == 0) {
							return ret;
						} else {
							ret = eh_execute(new_node, context);
						}
					}
					break;
				case T_RET: // return from a function or the program
					ret = eh_execute(node->get_opval()->paras[0], context);
					returning = true;
					break;
				case T_BREAK: // break out of a loop
					eh_op_break(node->get_opval(), context);
					break;
				case T_CONTINUE: // continue in a loop
					eh_op_continue(node->get_opval(), context);
					break;
			/*
			 * Object access
			 */
				case ':': // function call
					ret = eh_op_colon(node->get_opval()->paras, context);
					break;
			/*
			 * Object definitions
			 */
				case T_FUNC: // function definition
					ret = eh_op_declareclosure(node->get_opval()->paras, context);
					break;
				case T_CLASS: // class declaration
					ret = eh_op_declareclass(node->get_opval(), context);
					break;
				case T_CLASSMEMBER:
					eh_op_classmember(node->get_opval(), context);
					break;
				case T_INHERIT:
					eh_op_inherit(node->get_opval()->paras, context);
					break;
				case T_ATTRIBUTE: // class member attributes
					if(node->get_opval()->nparas == 0) {
						return ehretval_t::make_typed(attributestr_e);
					} else {
						// first execute first para
						ret = eh_execute(node->get_opval()->paras[0], context);
						// then overwrite with attribute from second para
						switch(node->get_opval()->paras[1]->attributeval) {
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
					ret = eh_op_array(node->get_opval()->paras[0], context);
					break;
				case '{': // anonymous class
					ret = eh_op_anonclass(node->get_opval()->paras[0], context);
					break;
			/*
			 * Binary operators
			 */
				case '.':
					return eh_op_dot(node->get_opval()->paras, context);
				case T_ARROW:
					return perform_op("operator_arrow", "operator->", 1, node->get_opval()->paras, context);
				case T_EQ: // strict equality
					operand1 = eh_execute(node->get_opval()->paras[0], context);
					operand2 = eh_execute(node->get_opval()->paras[1], context);
					return ehretval_t::make_bool(operand1->equals(operand2));
				case T_NE: // strict non-equality
					operand1 = eh_execute(node->get_opval()->paras[0], context);
					operand2 = eh_execute(node->get_opval()->paras[1], context);
					return ehretval_t::make_bool(!operand1->equals(operand2));
				EH_INTBOOL_CASE('>', >) // greater-than
				EH_INTBOOL_CASE('<', <) // lesser-than
				EH_INTBOOL_CASE(T_GE, >=) // greater-than or equal
				EH_INTBOOL_CASE(T_LE, <=) // lesser-than or equal
				case '+': // string concatenation, addition
					return perform_op("operator_plus", "operator+", 1, node->get_opval()->paras, context);
				case '-': // subtraction
					return perform_op("operator_minus", "operator-", 1, node->get_opval()->paras, context);
				case '*':
					return perform_op("operator_times", "operator*", 1, node->get_opval()->paras, context);
				case '/':
					return perform_op("operator_divide", "operator/", 1, node->get_opval()->paras, context);
				case '%':
					return perform_op("operator_modulo", "operator%", 1, node->get_opval()->paras, context);
				case '&':
				  return perform_op("operator_and", "operator&", 1, node->get_opval()->paras, context);
				case '^':
				  return perform_op("operator_xor", "operator^", 1, node->get_opval()->paras, context);
				case '|':
				  return perform_op("operator_or", "operator|", 1, node->get_opval()->paras, context);
				case T_AND: // AND; use short-circuit operation
					operand1 = eh_execute(node->get_opval()->paras[0], context);
					if(!this->to_bool(operand1, context)->get_boolval()) {
						return ehretval_t::make_bool(false);
					} else {
						operand2 = eh_execute(node->get_opval()->paras[1], context);
						return this->to_bool(operand2, context);
					}
				case T_OR: // OR; use short-circuit operation
					operand1 = eh_execute(node->get_opval()->paras[0], context);
					if(this->to_bool(operand1, context)->get_boolval()) {
						return ehretval_t::make_bool(true);
					} else {
						operand2 = eh_execute(node->get_opval()->paras[1], context);
						return this->to_bool(operand2, context);
					}
				case T_XOR:
					operand1 = eh_execute(node->get_opval()->paras[0], context);
					operand2 = eh_execute(node->get_opval()->paras[1], context);
					b1 = this->to_bool(operand1, context)->get_boolval();
					b2 = this->to_bool(operand2, context)->get_boolval();
					return ehretval_t::make_bool((b1 && !b2) || (!b1 && b2));
			/*
			 * Variable manipulation
			 */
				case T_RANGE:
					operand1 = eh_execute(node->get_opval()->paras[0], context);
					operand2 = eh_execute(node->get_opval()->paras[1], context);
					if(operand1->type() != operand2->type()) {
						eh_error("Incompatible types for range", enotice_e);
						return NULL;
					}
					ret = this->make_range(new ehrange_t(operand1, operand2));
					break;
				case T_SET:
					eh_op_set(node->get_opval()->paras, context);
					break;
				case T_MINMIN:
					operand1 = eh_execute(node->get_opval()->paras[0], context);
					switch(operand1->type()) {
						case int_e:
							operand1->intval--;
							break;
						default:
							eh_error_type("-- operator", operand1->type(), eerror_e);
							break;
					}
					break;
				case T_PLUSPLUS:
					operand1 = eh_execute(node->get_opval()->paras[0], context);
					switch(operand1->type()) {
						case int_e:
							operand1->intval++;
							break;
						default:
							eh_error_type("++ operator", operand1->type(), eerror_e);
							break;
					}
					break;
				case '$': // variable dereference
					ret = eh_op_dollar(node->get_opval()->paras[0], context);
					break;
			/*
			 * Commands
			 */
				case T_COMMAND:
					// name of command to be executed
					ret = eh_op_command(
						eh_execute(node->get_opval()->paras[0], context)->get_stringval(),
						node->get_opval()->paras[1],
						context
					);
					break;
				default:
					eh_error_int("Unexpected opcode", node->get_opval()->op, efatal_e);
					break;
			}
		} else {
			ret = node;
		}
	} catch(unknown_value_exception &e) {
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
	for( ; node->get_opval()->nparas != 0; node = node->get_opval()->paras[1]) {
		ehretval_p node2 = node->get_opval()->paras[0];
		if(node2->type() == op_e) {
			switch(node2->get_opval()->op) {
				case T_SHORTPARA:
					// short paras: set each short-form option to the same thing
					if(node2->get_opval()->nparas == 2) {
						// set to something else if specified
						value_r = eh_execute(node2->get_opval()->paras[1], context);
					} else {
						// set to true by default
						value_r = ehretval_t::make_bool(true);
					}
					node2 = eh_execute(node2->get_opval()->paras[0], context);
					for(int i = 0, len = strlen(node2->get_stringval()); i < len; i++) {
						char index[2];
						index[0] = node2->get_stringval()[i];
						index[1] = '\0';
						paras.string_indices[index] = value_r;
					}
					break;
				case T_LONGPARA:
				{
					// long-form paras
					char *index = eh_execute(node2->get_opval()->paras[0], context)->get_stringval();
					if(node2->get_opval()->nparas == 1) {
						paras.string_indices[index] = ehretval_t::make_bool(true);
					} else {
						paras.string_indices[index] = eh_execute(node2->get_opval()->paras[1], context);
					}
					break;
				}
				case T_REDIRECT:
					paras.string_indices[">"] = eh_execute(node2->get_opval()->paras[0], context);
					break;
				case '}':
					paras.string_indices["}"] = eh_execute(node2->get_opval()->paras[0], context);
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
	paras.string_indices["_ehphp"] = ehretval_t::make_bool(true);
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
	std::pair<int, int> range;

	// get the count
	ehretval_p count_r = eh_execute(op->paras[0], context);
	if(count_r->type() == range_e) {
		ehrange_t *rangeval = count_r->get_rangeval();
		if(rangeval->min->type() == int_e) {
			range.first = rangeval->min->get_intval();
			range.second = rangeval->max->get_intval();
		} else {
			eh_error_type("count", rangeval->min->type(), enotice_e);
			return NULL;
		}
	} else {
		count_r = this->to_int(count_r, context);
		if(count_r->type() != int_e) {
			eh_error_type("count", count_r->type(), eerror_e);
			return ret;
		}
		range.first = 0;
		range.second = count_r->get_intval() - 1;
	}
	if(op->nparas == 2) {
		// "for 5; do stuff; endfor" construct
		for(int i = range.first; i <= range.second; i++) {
			ret = eh_execute(op->paras[1], context);
			LOOPCHECKS;
		}
	} else {
		// "for 5 count i; do stuff; endfor" construct
		char *name = eh_execute(op->paras[1], context)->get_stringval();
		// this should perhaps create a new variable, or only overwrite variables in the current scope
		ehmember_p var = context->get_objectval()->get_recursive(name, context, T_LVALUE_SET);
		// if we do T_LVALUE_SET, get_recursive never returns NULL
		// count variable always gets to be an int
		var->value = ehretval_t::make_int(range.first);
		for( ; var->value->get_intval() <= range.second; var->value->intval++) {
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
	while(this->to_bool(eh_execute(paras[0], context), context)->get_boolval()) {
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
	if(object->type() != array_e && object->type() != object_e) {
		eh_error_type("for ... as operator", object->type(), enotice_e);
		return ret;
	}
	// increment loop count
	inloop++;
	// establish variables
	char *membername;
	ehmember_p membervar;
	char *indexname;
	ehmember_p indexvar;
	ehretval_p code;
	if(op->nparas == 3) {
		// no index
		membername = eh_execute(op->paras[1], context)->get_stringval();
		indexname = NULL;
		code = op->paras[2];
	} else {
		// with index
		indexname = eh_execute(op->paras[1], context)->get_stringval();
		membername = eh_execute(op->paras[2], context)->get_stringval();
		code = op->paras[3];
	}
	// create variables
	membervar = context->get_objectval()->get_recursive(membername, context, T_LVALUE_SET);
	if(indexname != NULL) {
		indexvar = context->get_objectval()->get_recursive(indexname, context, T_LVALUE_SET);
	}
	if(object->type() == object_e) {
		// check whether we're allowed to access private things
		const bool doprivate = object->get_objectval()->context_compare(context);
		OBJECT_FOR_EACH(object->get_objectval(), curr) {
			// ignore private
			if(!doprivate && curr->second->attribute.visibility == private_e) {
				continue;
			}
			membervar->value = curr->second->value;
			if(indexname) {
				indexvar->value = ehretval_t::make_string(strdup(curr->first.c_str()));
			}
			ret = eh_execute(code, context);
			LOOPCHECKS;
		
		}
	} else {
		// arrays
		eharray_t *array = object->get_arrayval();
		ARRAY_FOR_EACH_INT(array, i) {
			if(indexname) {
				indexvar->value = ehretval_t::make_int(i->first);
			}
			membervar->value = i->second;
			ret = eh_execute(code, context);
			LOOPCHECKS;
		}
		ARRAY_FOR_EACH_STRING(array, i) {
			if(indexname) {
				indexvar->value = ehretval_t::make_string(strdup(i->first.c_str()));
			}
			membervar->value = i->second;
			ret = eh_execute(code, context);
			LOOPCHECKS;		
		}
	}
	inloop--;
	return ret;
}
void EHI::eh_op_inherit(ehretval_p *paras, ehcontext_t context) {
	ehobj_t *classobj = this->get_class(eh_execute(paras[0], context), context);
	if(classobj != NULL) {
		OBJECT_FOR_EACH(classobj, i) {
			context->get_objectval()->copy_member(i, true, context, this);
		}
	}
}
void EHI::eh_op_break(opnode_t *op, ehcontext_t context) {
	int level;
	if(op->nparas == 0) {
		level = 1;
	} else {
		ehretval_p level_v = this->to_int(eh_execute(op->paras[0], context), context);
		if(level_v->type() != int_e) {
			return;
		} else {
			level = level_v->get_intval();
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
		ehretval_p level_v = this->to_int(eh_execute(op->paras[0], context), context);
		if(level_v->type() != int_e) {
			return;
		} else {
			level = level_v->get_intval();
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
	ehretval_p ret = this->make_array(new eharray_t);
	// need to count array members first, because they are reversed in our node.
	// That's not necessary with functions (where the situation is analogous), because the reversals that happen when parsing the prototype argument list and parsing the argument list in a call cancel each other out.
	int count = 0;
	for(ehretval_p node2 = node; node2->get_opval()->nparas != 0; node2 = node2->get_opval()->paras[0]) {
		count++;
	}
	for(ehretval_p node2 = node; node2->get_opval()->nparas != 0; node2 = node2->get_opval()->paras[0]) {
		array_insert(ret->get_arrayval(), node2->get_opval()->paras[1], --count, context);
	}
	return ret;
}
ehretval_p EHI::eh_op_anonclass(ehretval_p node, ehcontext_t context) {
	ehretval_p ret = this->make_hash(new ehhash_t());
	for( ; node->get_opval()->nparas != 0; node = node->get_opval()->paras[0]) {
		ehretval_p *myparas = node->get_opval()->paras[1]->get_opval()->paras;
		// nodes here will always have the name in para 0 and value in para 1
		ehretval_p namev = eh_execute(myparas[0], ret);
		if(namev->type() != string_e) {
			eh_error_type("hash member label", namev->type(), eerror_e);
			continue;
		}
		ehretval_p value = eh_execute(myparas[1], ret);
		ret->get_hashval()->set(namev->get_stringval(), value);
	}
	return ret;
}
ehretval_p EHI::eh_op_declareclosure(ehretval_p *paras, ehcontext_t context) {
	ehfunc_t *f = new ehfunc_t(user_e);
	ehretval_p object_data = ehretval_t::make_func(f);
	ehobj_t *function_object = new ehobj_t;
	ehretval_p ret = this->make_object(function_object);
	function_object->parent = context;
	function_object->type_id = func_e;
	function_object->object_data = object_data;
	f->code = paras[1];

	// insert "scope" pointer
	attributes_t thisattributes = attributes_t::make(private_e, nonstatic_e, const_e);
	ret->get_objectval()->insert_retval("scope", thisattributes, ret);

	// determine argument count
	f->argcount = count_nodes(paras[0]);
	// if there are no arguments, the arglist can be NULL
	if(f->argcount != 0) {
		f->args = new eharg_t[f->argcount]();
	}
	// add arguments to arglist
	int i = 0;
	for(ehretval_p tmp = paras[0]; tmp->get_opval()->nparas != 0;
		tmp = tmp->get_opval()->paras[0]) {
		f->args[i].name = eh_execute(tmp->get_opval()->paras[1], context)->get_stringval();
		i++;
	}
	return ret;
}
ehretval_p EHI::eh_op_declareclass(opnode_t *op, ehcontext_t context) {
	// create the ehretval_t
	ehretval_p ret = this->make_object(new ehobj_t());

	// process parameters
	ehretval_p code;
	int type_id;
	const char *name;
	if(op->nparas == 2) {
		name = eh_execute(op->paras[0], context)->get_stringval();
		type_id = this->repo.register_class(name, ret);
		code = op->paras[1];
	} else {
		type_id = base_object_e;
		code = op->paras[0];
	}

	ret->get_objectval()->type_id = type_id;
	ret->get_objectval()->parent = context;

	// insert "this" pointer
	attributes_t thisattributes = attributes_t::make(private_e, nonstatic_e, const_e);
	ret->get_objectval()->insert_retval("this", thisattributes, ret);

	// inherit from Object
	ehobj_t *object_class = this->repo.get_object(base_object_e)->get_objectval();
	OBJECT_FOR_EACH(object_class, member) {
		ret->get_objectval()->copy_member(member, false, ret, this);
	}

	eh_execute(code, ret);
	
	if(op->nparas == 2) {
		// insert variable
		ehmember_p member;
		member->value = ret;
		context->get_objectval()->insert(name, member);
	}
	return ret;
}
void EHI::eh_op_classmember(opnode_t *op, ehcontext_t context) {
	// rely on standard layout of the paras
	ehretval_p attribute_v = eh_execute(op->paras[0], context);
	attributes_t attribute = attribute_v->get_attributestrval();
	char *name = eh_execute(op->paras[1], context)->get_stringval();

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
	context->get_objectval()->insert_retval(name, attribute, value);
}
ehretval_p EHI::eh_op_switch(ehretval_p *paras, ehcontext_t context) {
	ehretval_p ret;
	// because we use continue, we'll pretend this is a loop
	inloop++;

	// switch variable
	ehretval_p switchvar = eh_execute(paras[0], context);
	for(ehretval_p node = paras[1]; node->get_opval()->nparas != 0; node = node->get_opval()->paras[1]) {
		opnode_t *op = node->get_opval()->paras[0]->get_opval();
		// execute default
		if(op->nparas == 1) {
			ret = eh_execute(op->paras[0], context);
		} else {
			ehretval_p casevar = eh_execute(op->paras[0], context);
			ehretval_p decider;
			// try to call function
			if(casevar->is_a(func_e) || casevar->type() == binding_e) {
				decider = call_function(casevar, 1, &switchvar, context);
				if(decider->type() != bool_e) {
					eh_error("Switch case method does not return bool", eerror_e);
					return NULL;
				}
			} else {
				decider = ehretval_t::make_bool(switchvar->equals(casevar));
			}
			// apply the decider
			if(decider->get_boolval()) {
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
	for(ehretval_p node = paras[1]; node->get_opval()->nparas != 0; node = node->get_opval()->paras[1]) {
		const opnode_t *op = node->get_opval()->paras[0]->get_opval();
		// execute default
		if(op->nparas == 1) {
			return eh_execute(op->paras[0], context);
		}
		ehretval_p casevar = eh_execute(op->paras[0], context);
		ehretval_p decider;
		if(casevar->is_a(func_e) || casevar->type() == binding_e) {
			decider = call_function(casevar, 1, &switchvar, context);
			if(decider->type() != bool_e) {
				eh_error("Given case method does not return bool", eerror_e);
				return NULL;
			}
		} else {
			decider = ehretval_t::make_bool(switchvar->equals(casevar));
		}
		if(decider->get_boolval()) {
			return eh_execute(op->paras[1], context);
		}
	}
	return NULL;
}
ehretval_p EHI::eh_op_colon(ehretval_p *paras, ehcontext_t context) {
	// parse arguments
	ehretval_p args = paras[1];
	int nargs = count_nodes(args);
	ehretval_p *new_args = new ehretval_p[nargs]();
	
	for(int i = 0; args->get_opval()->nparas != 0; args = args->get_opval()->paras[0], i++) {
		try {
			new_args[i] = eh_execute(args->get_opval()->paras[1], context);
		} catch(...) {
			delete[] new_args;
			throw;
		}
	}
	ehretval_p function = eh_execute(paras[0], context);
	ehretval_p ret;
	try {
		ret = call_function(function, nargs, new_args, context);
	} catch(...) {
		delete[] new_args;
		throw;
	}
	delete[] new_args;

	return ret;
}
ehretval_p EHI::eh_op_dollar(ehretval_p node, ehcontext_t context) {
	ehretval_p ret = eh_execute(node, context);
	if(ret == NULL) {
		return NULL;
	}
	
	ehretval_p varname = this->to_string(ret, context);
	if(varname == NULL) {
		return NULL;
	}
	
	ehmember_p var = context->get_objectval()->get_recursive(
		varname->get_stringval(), context, T_LVALUE_GET
	);
	if(var == NULL) {
		return NULL;
	} else {
		return var->value;
	}
}
ehretval_p EHI::eh_op_set(ehretval_p *paras, ehcontext_t context) {
	ehretval_p *internal_paras = paras[0]->get_opval()->paras;
	ehretval_p base_var = eh_execute(internal_paras[0], context);
	ehretval_p rvalue = eh_execute(paras[1], context);
	switch(paras[0]->get_opval()->op) {
		case T_ARROW: {
			//TODO: thread-safety or smart pointer
			ehretval_p *args = new ehretval_p[2]();
			args[0] = eh_execute(internal_paras[1], context);
			args[1] = rvalue;
			ehretval_p ret = call_method(base_var, "operator_arrow_equals", 2, args, context);
			delete[] args;
			return ret;
		}
		case '.': {
			// This is hard, since we will, for once, need to modify in-place. For now, only support objects. Functions too, just for fun.
			if(!base_var->is_object()) {
				eh_error_type("Cannot use operator.= on primitive", base_var->type(), enotice_e);
				return NULL;			
			}
			ehobj_t *obj = base_var->get_object();
			// accessor is guaranteed to be a string
			char *accessor = eh_execute(internal_paras[1], context)->get_stringval();
			obj->set(accessor, rvalue);
			return rvalue;
		}
		case '$': {
			ehmember_p var = context->get_objectval()->get_recursive(base_var->get_stringval(), context, T_LVALUE_SET);
			var->value = rvalue;
			return rvalue;
		}
	}
	assert(false);
	return NULL;
}
ehretval_p EHI::eh_op_dot(ehretval_p *paras, ehcontext_t context) {
	ehretval_p base_var = eh_execute(paras[0], context);
	const char *accessor = eh_execute(paras[1], context)->get_stringval();
	if(base_var->is_object()) {
		return base_var->get_object()->get(accessor, context, T_LVALUE_GET)->value;
	} else {
		ehobj_t *class_obj = this->get_primitive_class(base_var->type());
		if(class_obj == NULL) {
			eh_error_type("operator.", base_var->type(), enotice_e);
			return NULL;
		}
		if(class_obj->has(accessor)) {
			ehretval_p member = class_obj->get(accessor, context, T_LVALUE_GET)->value;
			if(member->is_a(func_e)) {
				return this->make_binding(new ehbinding_t(base_var, member));
			} else {
				return member;
			}
		} else {
			eh_error_unknown("object member", accessor, eerror_e);
			return NULL;
		}
	}
}
ehretval_p EHI::eh_op_try(ehretval_p *paras, ehcontext_t context) {
  ehretval_p ret;
  ehretval_p try_block = paras[0];
  ehretval_p catch_block = paras[1];
  ehretval_p finally_block = paras[2];
  try {
    try {
      ret = eh_execute(try_block, context);
    } catch(eh_exception& e) {
      // inject the exception into the current scope
      attributes_t attributes = attributes_t::make(public_e, nonstatic_e, nonconst_e);
      context->get_objectval()->insert_retval("exception", attributes, e.content);
      ret = eh_execute(catch_block, context);
    }
    eh_always_execute(finally_block, context);
  } catch(eh_exception &e) {
    eh_always_execute(finally_block, context);
    throw;
  }
  return ret;
}
ehretval_p EHI::eh_always_execute(ehretval_p code, ehcontext_t context) {
  // Execute even if we're breaking or continuing or whatever
  bool old_returning = returning;
  returning = false;
  int old_breaking = breaking;
  breaking = 0;
  int old_continuing = continuing;
  continuing = 0;
  ehretval_p ret = eh_execute(code, context);
  continuing = old_continuing;
  breaking = old_breaking;
  returning = old_returning;
  return ret;
}
// Perform an arbitrary operation defined as a method taking a single argument
ehretval_p EHI::perform_op(const char *name, const char *user_name, int nargs, ehretval_p *paras, ehcontext_t context) {
	ehretval_p base_var = eh_execute(paras[0], context);
	ehretval_p *args = NULL;
	if(nargs > 0) {
	  args = new ehretval_p[nargs]();
	  for(int i = 0; i < nargs; i++) {
	    args[i] = eh_execute(paras[i + 1], context);
	  }
	}
	ehretval_p ret = call_method(base_var, name, nargs, args, context);
	delete[] args;
	return ret;
}
/*
 * Functions
 */
ehretval_p EHI::call_method(ehretval_p obj, const char *name, int nargs, ehretval_p *args, ehcontext_t context) {
	ehretval_p func = NULL;
	if(obj->is_object()) {
		func = obj->get_object()->get(name, context, T_LVALUE_GET)->value;
	} else {
		ehobj_t *class_obj = this->get_primitive_class(obj->type());
		if(class_obj == NULL) {
			eh_error_type(name, obj->type(), enotice_e);
			return NULL;
		}
		ehmember_p method = class_obj->get(name, context, T_LVALUE_GET);
		if(method == NULL || !method->value->is_a(func_e)) {
		  // Don't throw an error, as it's likely to cascade
		  //eh_error_type("method", method->value->type(), enotice_e);
		  return NULL;
		}
    func = this->make_binding(new ehbinding_t(obj, method->value));
	}
	if(func == NULL) {
		return NULL;
	} else {
		return call_function(func, nargs, args, context);
	}
}
ehretval_p EHI::call_function(ehretval_p function, int nargs, ehretval_p *args, ehcontext_t context) {
	// We special-case function calls on func_e and binding_e types; otherwise we'd end up in an infinite loop
	if(function->type() == binding_e || function->is_a(func_e)) {
		// This one time, we call a library method directly. If you want to override Function.operator_colon, too bad.
		return ehlm_Function_operator_colon(NULL, nargs, args, function, this);
	} else if(function->type() == null_e) {
		// Silently ignore NULL
		return NULL;
	} else {
		return call_method(function, "operator_colon", nargs, args, context);
	}
}
/*
 * Classes
 */
ehretval_p EHI::object_instantiate(ehobj_t *obj, ehcontext_t context) {
	ehobj_t *new_obj = new ehobj_t();
	ehretval_p ret = this->make_object(new_obj);
	new_obj->type_id = obj->type_id;
	new_obj->parent = obj->parent;
	new_obj->real_parent = obj->real_parent;
	OBJECT_FOR_EACH(obj, m) {
		new_obj->copy_member(m, false, ret, this);
	}
	return ret;
}
ehobj_t *EHI::get_class(ehretval_p classname, ehcontext_t context) {
	ehobj_t *classobj;
	switch(classname->type()) {
		case string_e:
		{
			ehmember_p member = context->get_objectval()->get_recursive(classname->get_stringval(), context, T_LVALUE_GET);
			if(member == NULL) {
				eh_error_unknown("class", classname->get_stringval(), eerror_e);
				return NULL;
			}
			if(member->value->type() != object_e) {
				eh_error_type("class", member->value->type(), eerror_e);
				return NULL;
			}
			classobj = member->value->get_object();
			break;
		}
		case object_e:
			classobj = classname->get_objectval();
			break;
		default:
			eh_error_type("class name", classname->type(), eerror_e);
			return NULL;
	}
	return classobj;
}
// Promotes a pseudo-object of a builtin class to a real object
ehretval_p EHI::promote(ehretval_p in, ehcontext_t context) {
	ehobj_t *the_class = this->get_primitive_class(in->type());
	if(the_class == NULL) {
		eh_error_type("promotion", in->type(), enotice_e);
		return NULL;
	}
	ehretval_p obj = this->object_instantiate(the_class, context);
	obj->get_objectval()->object_data = in;
	return obj;
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
	if(in->get_opval()->nparas == 1) {
		// if there is no explicit key, simply use the place argument
		array->int_indices[place] = eh_execute(in->get_opval()->paras[0], context);
	} else {
		const ehretval_p label = eh_execute(in->get_opval()->paras[0], context);
		ehretval_p var = eh_execute(in->get_opval()->paras[1], context);
		switch(label->type()) {
			case int_e:
				array->int_indices[label->get_intval()] = var;
				break;
			case string_e:
				array->string_indices[label->get_stringval()] = var;
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
	argc_v->value = ehretval_t::make_int(argc - 1);
	global_object->get_objectval()->insert("argc", argc_v);

	// insert argv
	ehmember_p argv_v;
	argv_v->value = this->make_array(new eharray_t);

	// all members of argv are strings
	for(int i = 1; i < argc; i++) {
		argv_v->value->get_arrayval()->int_indices[i - 1] = ehretval_t::make_string(strdup(argv[i]));
	}
	global_object->get_objectval()->insert("argv", argv_v);
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

/*
 * Type casting
 */
ehretval_p EHI::eh_cast(const type_enum type, ehretval_p in, ehcontext_t context) {
	switch(type) {
		case int_e: return this->to_int(in, context);
		case string_e: return this->to_string(in, context);
		case float_e: return this->to_float(in, context);
		case bool_e: return this->to_bool(in, context);
		case array_e: return this->to_array(in, context);
		case range_e: return this->to_range(in, context);
		default:
			eh_error_type("typecast", type, eerror_e);
			break;
	}
	return NULL;
}

/*
 * Helper
 */
static inline int count_nodes(const ehretval_p node) {
	// count a list like an argument list. Assumes correct layout.
	int i = 0;
	for(ehretval_p tmp = node;
		tmp->get_opval()->nparas != 0;
		tmp = tmp->get_opval()->paras[0], i++
	);
	return i;
}
