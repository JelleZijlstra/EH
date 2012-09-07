/*
 * eh_interpreter.c
 * Jelle Zijlstra, December 2011
 *
 * Implements a Lex- and Yacc-based interpreter for the EH scripting language.
 */
#include <cctype>

#include "eh.h"
#include "eh_libclasses.h"
#include "eh_libcmds.h"
#include "std_lib/ArgumentError.h"
#include "std_lib/Array.h"
#include "std_lib/Bool.h"
#include "std_lib/ConstError.h"
#include "std_lib/CountClass.h"
#include "std_lib/Exception.h"
#include "std_lib/File.h"
#include "std_lib/Float.h"
#include "std_lib/Function.h"
#include "std_lib/GarbageCollector.h"
#include "std_lib/GlobalObject.h"
#include "std_lib/Hash.h"
#include "std_lib/Integer.h"
#include "std_lib/LoopError.h"
#include "std_lib/MiscellaneousError.h"
#include "std_lib/NameError.h"
#include "std_lib/Null.h"
#include "std_lib/Object.h"
#include "std_lib/Range.h"
#include "std_lib/String.h"
#include "std_lib/SuperClass.h"
#include "std_lib/SyntaxError.h"
#include "std_lib/Tuple.h"
#include "std_lib/TypeError.h"
#include "std_lib/UnknownCommandError.h"

typedef struct ehlc_listentry_t {
	const char *name;
	ehlm_listentry_t *members;
	int type_id;
} ehlc_listentry_t;
#define LIBCLASSENTRY(c, is_core) { #c, ehlc_l_ ## c, is_core},
ehlc_listentry_t libclasses[] = {
	LIBCLASSENTRY(Object, object_e)
	LIBCLASSENTRY(CountClass, -1)
	LIBCLASSENTRY(File, -1)
	LIBCLASSENTRY(Integer, int_e)
	LIBCLASSENTRY(String, string_e)
	LIBCLASSENTRY(Function, func_e)
	LIBCLASSENTRY(Array, array_e)
	LIBCLASSENTRY(Float, float_e)
	LIBCLASSENTRY(Bool, bool_e)
	LIBCLASSENTRY(Null, null_e)
	LIBCLASSENTRY(Range, range_e)
	LIBCLASSENTRY(Hash, hash_e)
	LIBCLASSENTRY(Tuple, tuple_e)
	LIBCLASSENTRY(SuperClass, super_class_e)
	LIBCLASSENTRY(Exception, -1)
	LIBCLASSENTRY(UnknownCommandError, -1)
	LIBCLASSENTRY(TypeError, -1)
	LIBCLASSENTRY(LoopError, -1)
	LIBCLASSENTRY(NameError, -1)
	LIBCLASSENTRY(ConstError, -1)
	LIBCLASSENTRY(ArgumentError, -1)
	LIBCLASSENTRY(SyntaxError, -1)
	LIBCLASSENTRY(GlobalObject, -1)
	LIBCLASSENTRY(MiscellaneousError, -1)
	LIBCLASSENTRY(GarbageCollector, -1)
	{NULL, NULL, 0}
};

typedef struct ehcmd_listentry_t {
	const char *name;
	ehlibmethod_t cmd;
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
EHI::EHI() : eval_parser(NULL), inloop(0), breaking(0), continuing(0), cmdtable(), buffer(NULL), gc(), returning(false), repo(), global_object() {
	eh_init();
}
void EHI::eh_init(void) {
	global_object = this->make_object(new ehobj_t());
	ehretval_p base_object = this->make_object(new ehobj_t());
	ehretval_p function_object = this->make_object(new ehobj_t());
	
	for(int i = 0; libclasses[i].name != NULL; i++) {
		ehobj_t *newclass;
		ehretval_p new_value;
		if(libclasses[i].type_id == object_e) {
			newclass = base_object->get_objectval();
			new_value = base_object;
		} else if(libclasses[i].type_id == func_e) {
			newclass = function_object->get_objectval();
			new_value = function_object;
		} else if(strcmp(libclasses[i].name, "GlobalObject") == 0) {
			newclass = global_object->get_objectval();
			new_value = global_object;
		} else {
			newclass = new ehobj_t();
			new_value = this->make_object(newclass);
		}
		// register class
		int type_id;
		if(libclasses[i].type_id == -1) {
			type_id = this->repo.register_class(libclasses[i].name, new_value);
		} else {
			type_id = libclasses[i].type_id;
			this->repo.register_known_class(type_id, libclasses[i].name, new_value);
		}
		newclass->type_id = type_id;
		if(strcmp(libclasses[i].name, "GlobalObject") != 0) {
			newclass->parent = global_object;
		}

		// inherit from Object, except in Object itself
		if(libclasses[i].type_id != object_e) {
			newclass->inherit(base_object);
		}
		ehlm_listentry_t *members = libclasses[i].members;
		// attributes for library methods
		attributes_t attributes = attributes_t::make(public_e, nonstatic_e, nonconst_e);
		for(int j = 0; members[j].name != NULL; j++) {
			ehretval_p func = make_method(members[j].func, function_object, new_value);
			ehmember_p func_member;
			func_member->attribute = attributes_t::make(public_e, nonstatic_e, nonconst_e);
			func_member->value = func;
			newclass->insert(members[j].name, func_member);
		}
		ehmember_p member;
		// library classes themselves are constant; otherwise the engine might blow up
		attributes.isconst = const_e;
		member->attribute = attributes;
		member->value = new_value;
		global_object->get_objectval()->insert(libclasses[i].name, member);
	}
	// insert global command table
	ehmember_p command_table;
	command_table->attribute = attributes_t::make(public_e, nonstatic_e, const_e);
	this->cmdtable = new ehhash_t();
	command_table->value = this->make_hash(this->cmdtable);
	// insert command table into global objects
	global_object->get_objectval()->insert("commands", command_table);

	// fill command table
	for(int i = 0; libcmds[i].name != NULL; i++) {
		ehretval_p cmd = make_method(libcmds[i].cmd, function_object, global_object);
		insert_command(libcmds[i].name, cmd);
	}
	for(int i = 0; libredirs[i][0] != NULL; i++) {
		redirect_command(libredirs[i][0], libredirs[i][1]);
	}
	// insert reference to global object
	ehmember_p global;
	global->attribute = attributes_t::make(public_e, nonstatic_e, const_e);
	global->value = global_object;
	global_object->get_objectval()->insert("global", global);

	gc.do_collect(global_object);
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
			  return perform_op("operator~", 0, node->get_opval()->paras, context);
			case T_NEGATIVE: // sign change
			  return perform_op("operator-", 0, node->get_opval()->paras, context);
			case '!': // Boolean not
			  return perform_op("operator!", 0, node->get_opval()->paras, context);
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
				return eh_op_while(node->get_opval()->paras, context);
			case T_FOR:
				return eh_op_for(node->get_opval(), context);
			case T_AS:
				return eh_op_as(node->get_opval(), context);
			case T_SWITCH: // switch statements
				ret = eh_op_switch(node->get_opval()->paras, context);
				// incremented in the eh_op_switch function
				inloop--;
				break;
			case T_GIVEN: // inline switch statements
				return eh_op_given(node->get_opval()->paras, context);
		/*
		 * Exceptions
		 */
			case T_TRY:
				return eh_op_try(node->get_opval(), context);
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
						return eh_execute(new_node, context);
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
				return eh_op_colon(node->get_opval()->paras, context);
			case T_THIS: // direct access to the context object
				return context.object;
			case T_SCOPE:
				return context.scope;
		/*
		 * Object definitions
		 */
			case T_FUNC: // function definition
				return eh_op_declareclosure(node->get_opval()->paras, context);
			case T_CLASS: // class declaration
				return eh_op_declareclass(node->get_opval(), context);
			case T_CLASSMEMBER:
				eh_op_classmember(node->get_opval(), context);
				break;
			case '[': // array declaration
				return eh_op_array(node->get_opval()->paras[0], context);
			case '{': // hash
				return eh_op_anonclass(node->get_opval()->paras[0], context);
				break;
			case ',': // tuple
				return eh_op_tuple(node, context);
			case T_RANGE:
				operand1 = eh_execute(node->get_opval()->paras[0], context);
				operand2 = eh_execute(node->get_opval()->paras[1], context);
				if(operand1->type() != operand2->type()) {
					throw_TypeError("Range members must have the same type", operand2->type(), this);
				}
				return this->make_range(new ehrange_t(operand1, operand2));
		/*
		 * Binary operators
		 */
			case '.':
				return eh_op_dot(node->get_opval()->paras, context);
			case T_ARROW:
				return perform_op("operator->", 1, node->get_opval()->paras, context);
			case T_EQ:
				return perform_op("operator==", 1, node->get_opval()->paras, context);
			case T_NE:
				return perform_op("operator!=", 1, node->get_opval()->paras, context);
			case '>':
				return perform_op("operator>", 1, node->get_opval()->paras, context);
			case T_GE:
				return perform_op("operator>=", 1, node->get_opval()->paras, context);
			case '<':
				return perform_op("operator<", 1, node->get_opval()->paras, context);
			case T_LE:
				return perform_op("operator<=", 1, node->get_opval()->paras, context);
			case T_COMPARE:
				return perform_op("operator<=>", 1, node->get_opval()->paras, context);
			case '+': // string concatenation, addition
				return perform_op("operator+", 1, node->get_opval()->paras, context);
			case '-': // subtraction
				return perform_op("operator-", 1, node->get_opval()->paras, context);
			case '*':
				return perform_op("operator*", 1, node->get_opval()->paras, context);
			case '/':
				return perform_op("operator/", 1, node->get_opval()->paras, context);
			case '%':
				return perform_op("operator%", 1, node->get_opval()->paras, context);
			case '&':
				return perform_op("operator&", 1, node->get_opval()->paras, context);
			case '^':
				return perform_op("operator^", 1, node->get_opval()->paras, context);
			case '|':
				return perform_op("operator|", 1, node->get_opval()->paras, context);
			case T_LEFTSHIFT:
				return perform_op("operator<<", 1, node->get_opval()->paras, context);
			case T_RIGHTSHIFT:
				return perform_op("operator>>", 1, node->get_opval()->paras, context);
			case '(':
				// this is to make nested tuples work
				return eh_execute(node->get_opval()->paras[0], context);
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
			case '=':
				return eh_op_set(node->get_opval()->paras, context);
			case '$': // variable dereference
				return eh_op_dollar(node->get_opval()->paras[0], context);
		/*
		 * Commands
		 */
			case T_COMMAND:
				// name of command to be executed
				return eh_op_command(
					eh_execute(node->get_opval()->paras[0], context)->get_stringval(),
					node->get_opval()->paras[1],
					context
				);
			default:
				std::cerr << "Unexpected opcode " << node->get_opval()->op;
				assert(false);
		}
	} else {
		ret = node;
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
	eharray_t *paras = new eharray_t();
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
						paras->string_indices[index] = value_r;
					}
					break;
				case T_LONGPARA: {
					// long-form paras
					char *index = eh_execute(node2->get_opval()->paras[0], context)->get_stringval();
					if(node2->get_opval()->nparas == 1) {
						paras->string_indices[index] = ehretval_t::make_bool(true);
					} else {
						paras->string_indices[index] = eh_execute(node2->get_opval()->paras[1], context);
					}
					break;
				}
				case T_REDIRECT:
					paras->string_indices[">"] = eh_execute(node2->get_opval()->paras[0], context);
					break;
				case '}':
					paras->string_indices["}"] = eh_execute(node2->get_opval()->paras[0], context);
					break;
				default: // non-named parameters with an expression
					paras->int_indices[count] = eh_execute(node2, context);
					count++;
					break;
			}
		} else {
			// non-named parameters
			paras->int_indices[count] = node2;
			count++;
		}
	}
	// insert indicator that this is an EH-PHP command
	paras->string_indices["_ehphp"] = ehretval_t::make_bool(true);
	// get the command to execute
	ehretval_p libcmd = get_command(name);
	ehretval_p ret;
	if(libcmd != NULL) {
		ehretval_p args = this->make_array(paras);
		ret = this->call_function(libcmd, args, context);
	} else {
		ret = execute_cmd(name, paras);
		delete paras;
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
			throw_TypeError("For loop counter must be an Integer or a Range of Integers", rangeval->min->type(), this);
		}
	} else if(count_r->type() == int_e) {
		range.first = 0;
		range.second = count_r->get_intval() - 1;
	} else {
		throw_TypeError("For loop counter must be an Integer or a Range of Integers", count_r->type(), this);
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
		ehmember_p var = this->set_property(context.scope, name, ehretval_t::make_int(range.first), context);
		for(int i = range.first; i <= range.second; i++) {
			var->value = ehretval_t::make_int(i);
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
		throw_TypeError("For-as loop", object->type(), this);
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
	membervar = this->set_property(context.scope, membername, ehretval_p(NULL), context);
	if(indexname != NULL) {
		indexvar = this->set_property(context.scope, indexname, ehretval_p(NULL), context);
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
void EHI::eh_op_break(opnode_t *op, ehcontext_t context) {
	int level;
	if(op->nparas == 0) {
		level = 1;
	} else {
		ehretval_p level_v = eh_execute(op->paras[0], context);
		if(level_v->type() != int_e) {
			throw_TypeError("break operator requires an Integer argument", level_v->type(), this);
		}
		level = level_v->get_intval();
	}
	// break as many levels as specified by the argument
	if(level > inloop) {
		throw_LoopError("break", level, this);
	}
	breaking = level;
	return;
}
void EHI::eh_op_continue(opnode_t *op, ehcontext_t context) {
	int level;
	if(op->nparas == 0) {
		level = 1;
	} else {
		ehretval_p level_v = eh_execute(op->paras[0], context);
		if(level_v->type() != int_e) {
			throw_TypeError("continue operator requires an Integer argument", level_v->type(), this);
		}
		level = level_v->get_intval();
	}
	// break as many levels as specified by the argument
	if(level > inloop) {
		throw_LoopError("continue", level, this);
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
	ehhash_t *new_hash = new ehhash_t();
	ehretval_p ret = this->make_hash(new_hash);
	for( ; node->get_opval()->nparas != 0; node = node->get_opval()->paras[0]) {
		ehretval_p *myparas = node->get_opval()->paras[1]->get_opval()->paras;
		// nodes here will always have the name in para 0 and value in para 1
		ehretval_p value = eh_execute(myparas[1], context);
		new_hash->set(myparas[0]->get_stringval(), value);
	}
	return ret;
}
ehretval_p EHI::eh_op_declareclosure(ehretval_p *paras, ehcontext_t context) {
	ehfunc_t *f = new ehfunc_t(user_e);
	ehretval_p object_data = ehretval_t::make_func(f);
	ehretval_p ret = this->get_primitive_class(func_e)->instantiate(this);
	ehobj_t *function_object = ret->get_objectval();
	function_object->parent = context.scope;
	function_object->type_id = func_e;
	function_object->object_data = object_data;
	f->code = paras[1];

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
		type_id = object_e;
		code = op->paras[0];
	}

	ret->get_objectval()->type_id = type_id;
	ret->get_objectval()->parent = context.scope;

	// inherit from Object
	ehretval_p object_class = this->repo.get_object(object_e);
	ret->get_objectval()->inherit(object_class);

	eh_execute(code, ehcontext_t(ret, ret));
	
	if(op->nparas == 2) {
		// insert variable
		ehmember_p member;
		member->value = ret;
		context.scope->get_objectval()->insert(name, member);
	}
	return ret;
}
ehretval_p EHI::eh_op_tuple(ehretval_p node, ehcontext_t context) {
	int nargs = 1;
	for(ehretval_p tmp = node;
		tmp->type() == op_e && tmp->get_opval()->op == ',' && tmp->get_opval()->nparas != 0;
		tmp = tmp->get_opval()->paras[1], nargs++
	) {}
	ehretval_a new_args(nargs);
	
	ehretval_p arg_node = node;
	for(int i = 0; i < nargs; i++) {
		opnode_t *op = arg_node->get_opval();
		if(op->op == ',') {
			new_args[i] = eh_execute(op->paras[0], context);
			arg_node = arg_node->get_opval()->paras[1];
		} else {
			new_args[i] = eh_execute(arg_node, context);
			assert(i == nargs - 1);
			break;
		}
	}
	return this->make_tuple(new ehtuple_t(nargs, new_args));
}
void EHI::eh_op_classmember(opnode_t *op, ehcontext_t context) {
	// rely on standard layout of the paras
	ehmember_p new_member;
	attributes_t attributes = attributes_t::make();
	for(ehretval_p node = op->paras[0]; node->get_opval()->nparas != 0; node = node->get_opval()->paras[0]) {
		switch(node->get_opval()->paras[1]->get_attributeval()) {
			case publica_e:
				attributes.visibility = public_e;
				break;
			case privatea_e:
				attributes.visibility = private_e;
				break;
			case statica_e:
				attributes.isstatic = static_e;
				break;
			case consta_e:
				attributes.isconst = const_e;
				break;
		}
	}
	new_member->attribute = attributes;
	char *name = eh_execute(op->paras[1], context)->get_stringval();

	// decide what we got
	switch(op->nparas) {
		case 2: // non-set property: null
			new_member->value = NULL;
			break;
		case 3: { // set property
			ehretval_p value = eh_execute(op->paras[2], context);
			if(value->type() == binding_e) {
				ehretval_p obj_data = value->get_bindingval()->object_data;
				if(obj_data->type() == object_e && obj_data->get_objectval() == context.scope->get_objectval()) {
					ehretval_p reference_retainer = value;
					value = value->get_bindingval()->method;
				}
			}
			new_member->value = value;
			break;
		}
	}
	this->set_member(context.scope, name, new_member, context);
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
				decider = call_function(casevar, switchvar, context);
				if(decider->type() != bool_e) {
					throw_TypeError("Method in a switch case must return a Bool", decider->type(), this);
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
			decider = call_function(casevar, switchvar, context);
			if(decider->type() != bool_e) {
				throw_TypeError("Method in a switch case must return a Bool", decider->type(), this);
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
	ehretval_p function = eh_execute(paras[0], context);
	ehretval_p args = eh_execute(paras[1], context);
	return call_function(function, args, context);
}
ehretval_p EHI::eh_op_dollar(ehretval_p node, ehcontext_t context) {
	ehretval_p varname = eh_execute(node, context);
	ehmember_p var = context.scope->get_objectval()->get_recursive(varname->get_stringval(), context);
	if(var == NULL) {
		throw_NameError(context.scope, varname->get_stringval(), this);
		return NULL;
	} else {
		return var->value;
	}
}
ehretval_p EHI::eh_op_set(ehretval_p *paras, ehcontext_t context) {
	ehretval_p rvalue = eh_execute(paras[1], context);
	return set(paras[0], rvalue, context);
}
ehretval_p EHI::set(ehretval_p lvalue, ehretval_p rvalue, ehcontext_t context) {
	ehretval_p *internal_paras = lvalue->get_opval()->paras;
	switch(lvalue->get_opval()->op) {
		case T_ARROW: {
			ehretval_p args[2];
			args[0] = eh_execute(internal_paras[1], context);
			args[1] = rvalue;
			ehretval_p base_var = eh_execute(internal_paras[0], context);
			return call_method(base_var, "operator->=", this->make_tuple(new ehtuple_t(2, args)), context);
		}
		case '.': {
			ehretval_p base_var = eh_execute(internal_paras[0], context);
			if(base_var->type() == super_class_e) {
				throw_TypeError("Cannot set member on parent class", super_class_e, this);
			}
			// This is hard, since we will, for once, need to modify in-place. For now, only support objects. Functions too, just for fun.
			if(!base_var->is_object()) {
				throw_TypeError("Cannot set member on primitive", base_var->type(), this);
			}
			// accessor is guaranteed to be a string
			char *accessor = eh_execute(internal_paras[1], context)->get_stringval();
			this->set_property(base_var, accessor, rvalue, context);
			return rvalue;
		}
		case '$': {
			ehretval_p base_var = eh_execute(internal_paras[0], context);
			const char *name = base_var->get_stringval();
			ehmember_p member = context.scope->get_objectval()->get_recursive(name, context);
			if(member != NULL && member->isconst()) {
				// bug: if the const member is actually in a higher scope, this error message will be wrong
				throw_ConstError(context.scope, name, this);
			}
			if(member == NULL) {
				this->set_property(context.scope, name, rvalue, context);
			} else {
				member->value = rvalue;
			}
			return rvalue;
		}
		case ',': {
			ehretval_p arg_node = lvalue;
			for(int i = 0; true; i++) {
				opnode_t *op = arg_node->get_opval();
				ehretval_p internal_rvalue = call_method(rvalue, "operator->", ehretval_t::make_int(i), context);
				if(op->op == ',') {
					set(op->paras[0], internal_rvalue, context);
					arg_node = arg_node->get_opval()->paras[1];
				} else {
					set(arg_node, internal_rvalue, context);
					break;
				}
			}
			return rvalue;
		}
		case '(':
			return set(internal_paras[0], rvalue, context);
		default:
			throw_MiscellaneousError("Invalid lvalue", this);
			break;
	}
	assert(false);
	return NULL;
}
ehretval_p EHI::eh_op_dot(ehretval_p *paras, ehcontext_t context) {
	ehretval_p base_var = eh_execute(paras[0], context);
	const char *accessor = eh_execute(paras[1], context)->get_stringval();
	if(base_var->type() == super_class_e) {
		return get_property(base_var->get_super_classval()->content(), accessor, context);
	} else {
		return get_property(base_var, accessor, context);
	}
}
ehretval_p EHI::eh_op_try(opnode_t *op, ehcontext_t context) {
	ehretval_p ret;
	ehretval_p try_block = op->paras[0];
	ehretval_p catch_blocks = op->paras[1];
	if(op->nparas == 2) {
		return eh_try_catch(try_block, catch_blocks, context);
	} else {
		ehretval_p finally_block = op->paras[2];
		ehretval_p ret;
		try {
			ret = eh_try_catch(try_block, catch_blocks, context);
		} catch(...) {
			eh_always_execute(finally_block, context);
			throw;
		}
		eh_always_execute(finally_block, context);
		return ret;
	}
}
ehretval_p EHI::eh_try_catch(ehretval_p try_block, ehretval_p catch_blocks, ehcontext_t context) {
	opnode_t *catch_op = catch_blocks->get_opval();
	// don't try/catch if there are no catch blocks
	if(catch_op->nparas == 0) {
		return eh_execute(try_block, context);
	}
	try {
		return eh_execute(try_block, context);	
	} catch(eh_exception &e) {
		// insert exception into current scope
		attributes_t attributes = attributes_t::make(public_e, nonstatic_e, nonconst_e);
		ehmember_p exception_member = ehmember_t::make(attributes, e.content);
		this->set_member(context.scope, "exception", exception_member, context);
		for(; catch_op->nparas != 0; catch_op = catch_op->paras[1]->get_opval()) {
			opnode_t *catch_block = catch_op->paras[0]->get_opval();
			if(catch_block->nparas == 1) {
				return eh_execute(catch_block->paras[0], context);
			} else {
				// conditional catch
				ehretval_p decider = eh_execute(catch_block->paras[0], context);
				if(decider->get_boolval()) {
					return eh_execute(catch_block->paras[1], context);
				}
			}
		}
		// re-throw if we couldn't catch it
		throw;
	}
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
ehretval_p EHI::perform_op(const char *name, int nargs, ehretval_p *paras, ehcontext_t context) {
	ehretval_p base_var = eh_execute(paras[0], context);
	ehretval_p args = NULL;
	if(nargs == 1) {
		args = eh_execute(paras[1], context);
	} else if(nargs > 1) {
		ehretval_a args_array(nargs);
		for(int i = 0; i < nargs; i++) {
			args_array[i] = eh_execute(paras[i + 1], context);
		}
		args = this->make_tuple(new ehtuple_t(nargs, args_array));
	}
	return call_method(base_var, name, args, context);
}
/*
 * Functions
 */
ehretval_p EHI::call_method(ehretval_p obj, const char *name, ehretval_p args, ehcontext_t context) {
	ehretval_p func = NULL;
	if(obj->is_object()) {
		func = this->get_property(obj, name, context);
	} else {
		ehretval_p the_property = this->get_property(obj, name, context);
		ehretval_p method;
		if(the_property->is_a(binding_e)) {
			method = the_property->get_bindingval()->method;
		} else {
			method = the_property;
		}
		if(!method->is_a(func_e)) {
			throw_TypeError("Method must be a function", method->type(), this);
		}
		func = this->make_binding(new ehbinding_t(obj, method));
	}
	if(func == NULL) {
		return NULL;
	} else {
		return call_function(func, args, context);
	}
}
ehretval_p EHI::call_function(ehretval_p function, ehretval_p args, ehcontext_t context) {
	// We special-case function calls on func_e and binding_e types; otherwise we'd end up in an infinite loop
	if(function->type() == binding_e || function->is_a(func_e)) {
		// This one time, we call a library method directly. If you want to override Function.operator_colon, too bad.
		return ehlm_Function_operator_colon(function, args, this);
	} else {
		return call_method(function, "operator:", args, context);
	}
}
/*
 * Classes
 */
ehmember_p EHI::set_property(ehretval_p object, const char *name, ehretval_p value, ehcontext_t context) {
	// caller should ensure object is actually an object
	ehobj_t *obj = object->get_objectval();
	// unbind bindings to myself
	if(value->type() == binding_e) {
		ehretval_p obj_data = value->get_bindingval()->object_data;
		if(obj_data->type() == object_e && obj == obj_data->get_objectval()) {
			ehretval_p reference_retainer = value;
			value = value->get_bindingval()->method;
		}
	}
	ehmember_p result = obj->inherited_get(name);
	if(ehmember_p::null(result)) {
		ehmember_p new_member;
		new_member->value = value;
		obj->insert(name, new_member);
		return new_member;		
	} else if(result->attribute.isconst == const_e) {
		throw_ConstError(object, name, this);
	} else if(result->attribute.visibility == private_e && !obj->context_compare(context)) {
		// pretend private members don't exist
		throw_NameError(object, name, this);
	} else if(result->attribute.isstatic == static_e) {
		result->value = value;
		return result;
	} else {
		// set in this object
		ehmember_p new_member;
		new_member->value = value;
		obj->insert(name, new_member);
		return new_member;
	}
	return result;
}
// insert an ehmember_p directly on this object, without worrying about inheritance
ehmember_p EHI::set_member(ehretval_p object, const char *name, ehmember_p value, ehcontext_t context) {
	// caller should ensure object is actually an object
	ehobj_t *obj = object->get_objectval();
	if(obj->has(name)) {
		ehmember_p the_member = obj->get_known(name);
		if(the_member->attribute.isconst == const_e) {
			throw_ConstError(object, name, this);
		}
		if(the_member->attribute.visibility == private_e && !obj->context_compare(context)) {
			// pretend private members don't exist
			throw_NameError(object, name, this);
		}
	}
	obj->insert(name, value);
	return value;
}
ehretval_p EHI::get_property(ehretval_p base_var, const char *name, ehcontext_t context) {
	ehretval_p object;
	if(base_var->is_object()) {
		object = base_var;
	} else {
		object = this->get_primitive_class(base_var->type());
	}
	ehobj_t *obj = object->get_objectval();
	ehmember_p member = obj->inherited_get(name);
	if(ehmember_p::null(member) || (member->attribute.visibility == private_e && !obj->context_compare(context))) {
		throw_NameError(base_var, name, this);		
	}
	ehretval_p out = member->value;
	if(out->is_a(func_e)) {
		return this->make_binding(new ehbinding_t(base_var, out));
	} else {
		return out;
	}
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
				throw_TypeError_Array_key(label->type(), this);
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
ehretval_p EHI::get_command(const char *name) {
	if(this->cmdtable->has(name)) {
		return this->cmdtable->get(name);
	} else {
		return NULL;
	}
}
void EHI::insert_command(const char *name, ehretval_p cmd) {
	this->cmdtable->set(name, cmd);
}
void EHI::redirect_command(const char *redirect, const char *target) {
	ehretval_p targetcmd = get_command(target);
	if(targetcmd == NULL) {
		throw_UnknownCommandError(target, this);
	}
	insert_command(redirect, targetcmd);
}
/*
 * Exceptions
 */
void EHI::handle_uncaught(eh_exception &e) {
	try {
		ehretval_p content = e.content;
		int type = content->get_full_type();
		const std::string &type_string = this->repo.get_name(type);
		// we're in global context now. Remember this object, because otherwise the string may be freed before we're done with it.
		ehretval_p stringval = this->to_string(content, ehcontext_t(this->global_object, this->global_object));
		const char *msg = stringval->get_stringval();
		std::cerr << "Uncaught exception of type " << type_string << ": " << msg << std::endl;
	} catch(...) {
		std::cerr << "Exception occurred while handling uncaught exception" << std::endl;
	}
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
		default: throw_TypeError("Cannot use this type with global cast operator", type, this);
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
	) {}
	return i;
}
