/*
 * eh_interpreter.c
 * Jelle Zijlstra, December 2011
 *
 * Implements a Lex- and Yacc-based interpreter for the EH scripting language.
 */
#include <cctype>
#include <string.h>

#include "eh.hpp"
#include "eh_libclasses.hpp"
#include "eh_libcmds.hpp"
#include "std_lib/ConstError.hpp"
#include "std_lib/Enum.hpp"
#include "std_lib/GlobalObject.hpp"
#include "std_lib/LoopError.hpp"
#include "std_lib/MiscellaneousError.hpp"
#include "std_lib/NameError.hpp"
#include "std_lib/SuperClass.hpp"
#include "std_lib/VisibilityError.hpp"

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
static attributes_t parse_attributes(ehretval_p node);

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
EHInterpreter::EHInterpreter() : gc(), repo(), global_object(), function_object(), base_object(), included_files() {
	eh_init();
}
void EHInterpreter::eh_init(void) {
	ehobj_t *global_ehobj = new ehobj_t();
	global_object = this->make_object(global_ehobj);

	// manually register GlobalObject in itself, run initializer
	global_ehobj->register_member_class("GlobalObject", -1, ehinit_GlobalObject, attributes_t::make_const(), this, global_object);

	// insert global command table
	ehmember_p command_table;
	command_table->attribute = attributes_t::make_const();
	this->cmdtable = new ehhash_t();
	command_table->value = this->make_hash(this->cmdtable);
	// insert command table into global objects
	global_object->get_objectval()->insert("commands", command_table);

	// fill command table
	for(int i = 0; libcmds[i].name != NULL; i++) {
		ehretval_p cmd = make_method(libcmds[i].cmd);
		insert_command(libcmds[i].name, cmd);
	}
	for(int i = 0; libredirs[i][0] != NULL; i++) {
		redirect_command(libredirs[i][0], libredirs[i][1]);
	}

	gc.do_collect(global_object);
}
void EHInterpreter::eh_exit(void) {
	this->global_object->get_objectval()->members.erase("global");
	this->gc.do_collect(this->global_object);
	return;
}
EHInterpreter::~EHInterpreter() {
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
		opnode_t *op = node->get_opval();
		if(op == NULL) {
			return NULL;
		}
		ehretval_p *paras = op->paras;
		switch(op->op) {
			case T_LITERAL:
				return paras[0];
			case T_NULL:
				return NULL;
			case '_':
				throw_MiscellaneousError("Cannot use _ in expression", this);
				return NULL;
		/*
		 * Unary operators
		 */
			case '@': // type casting
				throw_MiscellaneousError("Cannot use @ outside of match statement", this);
				break;
			case '~': // bitwise negation
			  return perform_op("operator~", 0, paras, context);
			case T_NEGATIVE: // sign change
			  return perform_op("operator-", 0, paras, context);
			case '!': // Boolean not
			  return perform_op("operator!", 0, paras, context);
		/*
		 * Control flow
		 */
			case T_IF:
				return eh_op_if(op, context);
			case T_WHILE:
				return eh_op_while(paras, context);
			case T_IN:
				return eh_op_in(op, context);
			case T_SWITCH: // switch statements
				ret = eh_op_switch(paras, context);
				// incremented in the eh_op_switch function
				inloop--;
				break;
			case T_GIVEN: // inline switch statements
				return eh_op_given(paras, context);
			case T_MATCH:
				return eh_op_match(paras, context);
		/*
		 * Exceptions
		 */
			case T_TRY:
				return eh_op_try(op, context);
		/*
		 * Miscellaneous
		 */
			case T_SEPARATOR:
				// execute both commands
				ret = eh_execute(paras[0], context);
				if(returning || breaking || continuing) {
					return ret;
				} else {
					return eh_execute(paras[1], context);
				}
			case T_RET: // return from a function or the program
				if(op->nparas == 0) {
					ret = NULL;
				} else {
					ret = eh_execute(paras[0], context);
				}
				returning = true;
				break;
			case T_BREAK: // break out of a loop
				eh_op_break(op, context);
				break;
			case T_CONTINUE: // continue in a loop
				eh_op_continue(op, context);
				break;
		/*
		 * Object access
		 */
			case ':': // function call
				return eh_op_colon(paras, context);
			case T_CALL_METHOD: {
				ehretval_p obj = eh_execute(paras[0], context);
				ehretval_p arg = eh_execute(paras[2], context);
				return call_method(obj, paras[1]->get_stringval(), arg, context);
			}
			case T_THIS: // direct access to the context object
				return context.object;
			case T_SCOPE:
				return context.scope;
		/*
		 * Object definitions
		 */
			case T_FUNC: // function definition
				return eh_op_declareclosure(paras, context);
			case T_CLASS: // class declaration
				return eh_op_declareclass(op, context);
			case T_CLASSMEMBER:
				return eh_op_classmember(op, context);
			case T_ENUM:
				return eh_op_enum(op, context);
			case '[': // array declaration
				return eh_op_array(paras[0], context);
			case '{': // hash
				return eh_op_anonclass(paras[0], context);
			case ',': // tuple
				return eh_op_tuple(node, context);
			case T_RANGE:
				operand1 = eh_execute(paras[0], context);
				operand2 = eh_execute(paras[1], context);
				if(operand1->type() != operand2->type()) {
					throw_TypeError("Range members must have the same type", operand2->type(), this);
				}
				return parent->make_range(new ehrange_t(operand1, operand2));
		/*
		 * Binary operators
		 */
			case '.':
				return eh_op_dot(paras, context);
			case T_ARROW:
				return perform_op("operator->", 1, paras, context);
			case T_EQ:
				return perform_op("operator==", 1, paras, context);
			case T_NE:
				return perform_op("operator!=", 1, paras, context);
			case '>':
				return perform_op("operator>", 1, paras, context);
			case T_GE:
				return perform_op("operator>=", 1, paras, context);
			case '<':
				return perform_op("operator<", 1, paras, context);
			case T_LE:
				return perform_op("operator<=", 1, paras, context);
			case T_COMPARE:
				return perform_op("operator<=>", 1, paras, context);
			case '+': // string concatenation, addition
				return perform_op("operator+", 1, paras, context);
			case '-': // subtraction
				return perform_op("operator-", 1, paras, context);
			case '*':
				return perform_op("operator*", 1, paras, context);
			case '/':
				return perform_op("operator/", 1, paras, context);
			case '%':
				return perform_op("operator%", 1, paras, context);
			case '&':
				return perform_op("operator&", 1, paras, context);
			case '^':
				return perform_op("operator^", 1, paras, context);
			case '|':
				return perform_op("operator|", 1, paras, context);
			case T_LEFTSHIFT:
				return perform_op("operator<<", 1, paras, context);
			case T_RIGHTSHIFT:
				return perform_op("operator>>", 1, paras, context);
			case '(':
				// this is to make nested tuples work
				return eh_execute(paras[0], context);
			case T_AND: // AND; use short-circuit operation
				operand1 = eh_execute(paras[0], context);
				if(!this->to_bool(operand1, context)->get_boolval()) {
					return ehretval_t::make_bool(false);
				} else {
					operand2 = eh_execute(paras[1], context);
					return this->to_bool(operand2, context);
				}
			case T_OR: // OR; use short-circuit operation
				operand1 = eh_execute(paras[0], context);
				if(this->to_bool(operand1, context)->get_boolval()) {
					return ehretval_t::make_bool(true);
				} else {
					operand2 = eh_execute(paras[1], context);
					return this->to_bool(operand2, context);
				}
			case T_XOR:
				operand1 = eh_execute(paras[0], context);
				operand2 = eh_execute(paras[1], context);
				b1 = this->to_bool(operand1, context)->get_boolval();
				b2 = this->to_bool(operand2, context)->get_boolval();
				return ehretval_t::make_bool((b1 && !b2) || (!b1 && b2));
			case T_CUSTOMOP:
				return eh_op_customop(paras, context);
		/*
		 * Variable manipulation
		 */
			case '=':
				return eh_op_set(paras, context);
			case '$': // variable dereference
				return eh_op_dollar(paras[0], context);
		/*
		 * Commands
		 */
			case T_COMMAND:
				return eh_op_command(
					paras[0]->get_stringval(),
					paras[1],
					context
				);
			default:
				std::cerr << "Unexpected opcode " << op->op;
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
		// every para_expr should have an op associated with it
		assert(node2->type() == op_e);
		ehretval_p *node_paras = node2->get_opval()->paras;
		switch(node2->get_opval()->op) {
			case T_SHORTPARA: {
				// short paras: set each short-form option to the same thing
				if(node2->get_opval()->nparas == 2) {
					// set to something else if specified
					value_r = eh_execute(node_paras[1], context);
				} else {
					// set to true by default
					value_r = ehretval_t::make_bool(true);
				}
				const char *shorts = node_paras[0]->get_stringval();
				for(int i = 0, len = strlen(shorts); i < len; i++) {
					char index[2];
					index[0] = shorts[i];
					index[1] = '\0';
					paras->string_indices[index] = value_r;
				}
				break;
			}
			case T_LONGPARA: {
				// long-form paras
				const char *index = node_paras[0]->get_stringval();
				if(node2->get_opval()->nparas == 1) {
					paras->string_indices[index] = ehretval_t::make_bool(true);
				} else {
					paras->string_indices[index] = eh_execute(node_paras[1], context);
				}
				break;
			}
			case T_REDIRECT:
				paras->string_indices[">"] = eh_execute(node_paras[0], context);
				break;
			case '}':
				paras->string_indices["}"] = eh_execute(node_paras[0], context);
				break;
			default: // non-named parameters with an expression
				paras->int_indices[count] = eh_execute(node2, context);
				count++;
				break;
		}
	}
	// insert indicator that this is an EH-PHP command
	paras->string_indices["_ehphp"] = ehretval_t::make_bool(true);
	// get the command to execute
	ehretval_p libcmd = parent->get_command(name);
	ehretval_p ret;
	if(libcmd != NULL) {
		ehretval_p args = parent->make_array(paras);
		ret = this->call_function(libcmd, args, context);
	} else {
		try {
			ret = this->execute_cmd(name, paras);
		} catch(...) {
			delete paras;
			throw;
		}
		delete paras;
	}
	// we're not returning anymore
	returning = false;
	return ret;
}
ehretval_p EHI::eh_op_if(opnode_t *op, ehcontext_t context) {
	if(this->to_bool(eh_execute(op->paras[0], context), context)->get_boolval()) {
		return eh_execute(op->paras[1], context);
	} else if(op->nparas == 2) {
		// if something { do_something() }
		return NULL;
	} else {
		// loop through elsifs
		for(opnode_t *iop = op->paras[2]->get_opval(); iop->nparas != 0; iop = iop->paras[1]->get_opval()) {
			ehretval_p *current_block = iop->paras[0]->get_opval()->paras;
			if(this->to_bool(eh_execute(current_block[0], context), context)->get_boolval()) {
				return eh_execute(current_block[1], context);
			}
		}
		// if there is a final else
		if(op->nparas == 4) {
			return eh_execute(op->paras[3], context);
		} else {
			return NULL;
		}
	}
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
ehretval_p EHI::eh_op_in(opnode_t *op, ehcontext_t context) {
	ehretval_p iteree_block, body_block;
	if(op->nparas == 3) {
		iteree_block = op->paras[1];
		body_block = op->paras[2];
	} else {
		iteree_block = op->paras[0];
		body_block = op->paras[1];
	}
	ehretval_p iteree = eh_execute(iteree_block, context);
	ehretval_p iterator = call_method(iteree, "getIterator", NULL, context);
	inloop++;
	while(true) {
		ehretval_p has_next = call_method(iterator, "hasNext", NULL, context);
		if(has_next->type() != bool_e) {
			throw_TypeError("hasNext does not return a bool", has_next->type(), this);
		}
		if(!has_next->get_boolval()) {
			break;
		}
		ehretval_p next = call_method(iterator, "next", NULL, context);
		if(op->nparas == 3) {
			attributes_t attributes = attributes_t::make(private_e, nonstatic_e, nonconst_e);
			set(op->paras[0], next, &attributes, context);
		}
		ehretval_p ret = eh_execute(body_block, context);
		LOOPCHECKS;
	}
	inloop--;
	return iteree;
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
	ehretval_p ret = parent->make_array(new eharray_t);
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
	ehretval_p ret = parent->make_hash(new_hash);
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
	ehretval_p ret = parent->instantiate(parent->get_primitive_class(func_e));
	ehobj_t *function_object = ret->get_objectval();
	function_object->parent = context.scope;
	function_object->type_id = func_e;
	function_object->object_data = ehretval_t::make_func(f);
	f->code = paras[1];
	f->args = paras[0];
	return ret;
}
ehretval_p EHI::eh_op_enum(opnode_t *op, ehcontext_t context) {
	// unpack arguments
	const char *name = op->paras[0]->get_stringval();
	ehretval_p members_code = op->paras[1];
	ehretval_p code = op->paras[2];

	// create Enum object
	ehretval_p ret = Enum::make(name, this);
	Enum *e = Enum::extract_enum(ret);

	// extract enum members
	for(ehretval_p node = members_code; ; node = node->get_opval()->paras[0]) {
		ehretval_p current_member;
		bool is_last;
		if(node->get_opval()->op == ',') {
			current_member = node->get_opval()->paras[1];
			is_last = false;
		} else {
			current_member = node;
			is_last = true;
		}

		// handle the member
		const char *member_name = current_member->get_opval()->paras[0]->get_stringval();
		if(current_member->get_opval()->nparas == 1) {
			Enum::add_nullary_member(ret, member_name, this);
		} else {
			std::vector<std::string> params(0);
			for(ehretval_p argument = current_member->get_opval()->paras[1]; ; argument = argument->get_opval()->paras[1]) {
				const char *name = argument->get_opval()->paras[0]->get_stringval();
				params.push_back(name);
				if(argument->get_opval()->op != ',') {
					break;
				}
			}
			Enum::add_member_with_arguments(ret, member_name, params, this);
		}

		if(is_last) {
			break;
		}
	}

	// execute internal code
	eh_execute(code, ehcontext_t(ret, e->contents));

	// remove inheritance (terrible hack)
	e->contents->get_objectval()->super.clear();

	// insert variable
	ehmember_p member;
	member->value = ret;
	context.scope->get_objectval()->insert(name, member);
	return ret;
}
ehretval_p EHI::eh_op_declareclass(opnode_t *op, ehcontext_t context) {
	// create the ehretval_t
	ehobj_t *new_obj = new ehobj_t();
	ehretval_p ret = parent->make_object(new_obj);

	// process parameters
	ehretval_p code;
	const char *name = "(anonymous class)";
	if(op->nparas == 2) {
		// named class
		name = op->paras[0]->get_stringval();
		code = op->paras[1];
	} else {
		// nameless class
		code = op->paras[0];
	}

	new_obj->type_id = parent->repo.register_class(name, ret);
	new_obj->parent = context.scope;

	// inherit from Object
	new_obj->inherit(parent->base_object);

	// execute the code within the class
	eh_execute(code, ret);

	if(op->nparas == 2) {
		// insert variable if it is a named class
		ehmember_p member;
		member->value = ret;
		context.scope->get_objectval()->insert(name, member);
#if 0
		// set class's own name property (commenting out until we can distinguish classes and prototypes better)
		ehmember_p name_member;
		name_member->value = ehretval_t::make_string(strdup(name));
		new_obj->insert("name", name_member);
#endif
	}
	return ret;
}
ehretval_p EHI::eh_op_tuple(ehretval_p node, ehcontext_t context) {
	int nargs = 1;
	// first determine the size of the tuple
	for(ehretval_p tmp = node;
		tmp->type() == op_e && tmp->get_opval()->op == ',' && tmp->get_opval()->nparas != 0;
		tmp = tmp->get_opval()->paras[1], nargs++
	) {}
	ehretval_a new_args(nargs);

	ehretval_p arg_node = node;
	// now, fill the output tuple
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
	return parent->make_tuple(new ehtuple_t(nargs, new_args));
}
ehretval_p EHI::eh_op_classmember(opnode_t *op, ehcontext_t context) {
	// parse the attributes into an attributes_t
	attributes_t attributes = parse_attributes(op->paras[0]);
	// set the member
	return set(op->paras[1], NULL, &attributes, context);
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
			decider = call_method(switchvar, "operator==", casevar, context);
			if(decider->type() != bool_e) {
				throw_TypeError("operator== does not return a bool", decider->type(), this);
			}
		}
		if(decider->get_boolval()) {
			return eh_execute(op->paras[1], context);
		}
	}
	throw_MiscellaneousError("No matching case in given statement", this);
	return NULL;
}

bool EHI::match(ehretval_p node, ehretval_p var, ehcontext_t context) {
	opnode_t *op = node->get_opval();
	switch(op->op) {
		case '_':
			return true;
		case '@': {
			const char *name = op->paras[0]->get_stringval();
			attributes_t attributes = attributes_t::make(private_e, nonstatic_e, nonconst_e);
			ehmember_p member;
			member->attribute = attributes;
			member->value = var;
			this->set_member(context.scope, name, member, context);
			return true;
		}
		case '|': {
			return match(op->paras[0], var, context) || match(op->paras[1], var, context);
		}
		case ',': {
			if(var->type() != tuple_e) {
				return false;
			}
			ehtuple_t *t = var->get_tupleval();
			const int size = t->size();
			int i = 0;
			for(ehretval_p arg_node = node; ; arg_node = arg_node->get_opval()->paras[1], i++) {
				if(i == size) {
					return false;
				}
				opnode_t *op = arg_node->get_opval();
				if(op->op == ',') {
					if(!match(op->paras[0], t->get(i), context)) {
						return false;
					}
				} else {
					return match(arg_node, t->get(i), context);
				}
			}
		}
		case ':': {
			ehretval_p member = eh_execute(op->paras[0], context);
			if(!member->is_a(parent->enum_member_id)) {
				throw_TypeError("match case is not an Enum.Member", member->type(), this);
			}
			ehretval_p member_em = ehretval_t::self_or_data(member);
			Enum_Member *em = static_cast<Enum_Member *>(member_em->get_resourceval());

			if(!var->is_a(parent->enum_instance_id)) {
				return false;
			}
			var = ehretval_t::self_or_data(var);
			Enum_Instance *var_ei = static_cast<Enum_Instance *>(var->get_resourceval());

			if(member_em->naive_compare(var_ei->member()) != 0) {
				return false;
			}
			int size = em->size;
			if(op->paras[1]->get_opval()->op != '(') {
				throw_MiscellaneousError("Invalid argument in Enum.Member match", this);
			}
			int nargs = 1;
			ehretval_p args = op->paras[1]->get_opval()->paras[0];
			for(ehretval_p tmp = args;
				tmp->type() == op_e && tmp->get_opval()->op == ',' && tmp->get_opval()->nparas != 0;
				tmp = tmp->get_opval()->paras[1], nargs++
			);
			if(nargs != size) {
				throw_MiscellaneousError("Invalid argument number in Enum.Member match", this);
			}
			ehretval_p arg_node = args;
			for(int i = 0; i < nargs; i++) {
				opnode_t *op = arg_node->get_opval();
				if(op->op == ',') {
					if(!match(op->paras[0], var_ei->get(i), context)) {
						return false;
					}
					arg_node = arg_node->get_opval()->paras[1];
				} else {
					if(!match(arg_node, var_ei->get(i), context)) {
						return false;
					}
					assert(i == nargs - 1);
					break;
				}
			}
			return true;
		}
		case '(': {
			return match(op->paras[0], var, context);
		}
		default: {
			ehretval_p casevar = eh_execute(node, context);
			ehretval_p decider = call_method(var, "operator==", casevar, context);
			if(decider->type() != bool_e) {
				throw_TypeError("operator== does not return a bool", decider->type(), this);
			}
			return decider->get_boolval();
		}
	}
}

ehretval_p EHI::eh_op_match(ehretval_p *paras, ehcontext_t context) {
	// switch variable
	ehretval_p switchvar = eh_execute(paras[0], context);
	for(ehretval_p node = paras[1]; node->get_opval()->nparas != 0; node = node->get_opval()->paras[1]) {
		ehretval_p case_node = node->get_opval()->paras[0];
		if(match(case_node->get_opval()->paras[0], switchvar, context)) {
			return eh_execute(case_node->get_opval()->paras[1], context);
		}
	}
	throw_MiscellaneousError("No matching case in match statement", this);
	return NULL;
}
ehretval_p EHI::eh_op_customop(ehretval_p *paras, ehcontext_t context) {
	ehretval_p lhs = eh_execute(paras[0], context);
	ehretval_p rhs = eh_execute(paras[2], context);
	std::string op = paras[1]->get_stringval();
	return call_method(lhs, ("operator" + op).c_str(), rhs, context);
}
ehretval_p EHI::eh_op_colon(ehretval_p *paras, ehcontext_t context) {
	// parse arguments
	ehretval_p function = eh_execute(paras[0], context);
	ehretval_p args = eh_execute(paras[1], context);
	return call_function(function, args, context);
}
ehretval_p EHI::eh_op_dollar(ehretval_p node, ehcontext_t context) {
	ehmember_p var = context.scope->get_objectval()->get_recursive(node->get_stringval(), context);
	if(var == NULL) {
		throw_NameError(context.scope, node->get_stringval(), this);
		return NULL;
	} else {
		return var->value;
	}
}
ehretval_p EHI::eh_op_set(ehretval_p *paras, ehcontext_t context) {
	ehretval_p rvalue = eh_execute(paras[1], context);
	return set(paras[0], rvalue, NULL, context);
}
ehretval_p EHI::set(ehretval_p lvalue, ehretval_p rvalue, attributes_t *attributes, ehcontext_t context) {
	ehretval_p *internal_paras = lvalue->get_opval()->paras;
	switch(lvalue->get_opval()->op) {
		case T_ARROW: {
			ehretval_p args[2];
			args[0] = eh_execute(internal_paras[1], context);
			args[1] = rvalue;
			ehretval_p base_var = eh_execute(internal_paras[0], context);
			return call_method(base_var, "operator->=", parent->make_tuple(new ehtuple_t(2, args)), context);
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
			char *accessor = internal_paras[1]->get_stringval();
			if(attributes == NULL) {
				this->set_property(base_var, accessor, rvalue, context);
			} else {
				ehmember_p new_member;
				new_member->value = rvalue;
				new_member->attribute = *attributes;
				this->set_member(base_var, accessor, new_member, context);
			}
			return rvalue;
		}
		case '$': {
			const char *name = internal_paras[0]->get_stringval();
			attributes_t attributes_container = attributes_t::make();
			if(attributes == NULL) {
				ehmember_p member = context.scope->get_objectval()->get_recursive(name, context);
				if(member != NULL) {
					if(member->isconst()) {
						// bug: if the const member is actually in a higher scope, this error message will be wrong
						throw_ConstError(context.scope, name, this);
					}
					member->value = rvalue;
					return rvalue;
				}
				attributes = &attributes_container;
			}
			ehmember_p new_member;
			new_member->value = rvalue;
			new_member->attribute = *attributes;
			this->set_member(context.scope, name, new_member, context);
			return rvalue;
		}
		case ',': {
			ehretval_p arg_node = lvalue;
			for(int i = 0; true; i++) {
				opnode_t *op = arg_node->get_opval();
				ehretval_p internal_rvalue;
				if(rvalue->type() != null_e) {
					internal_rvalue = call_method(rvalue, "operator->", ehretval_t::make_int(i), context);
				} else {
					internal_rvalue = NULL;
				}
				if(op->op == ',') {
					set(op->paras[0], internal_rvalue, attributes, context);
					arg_node = op->paras[1];
				} else {
					set(arg_node, internal_rvalue, attributes, context);
					break;
				}
			}
			return rvalue;
		}
		case '(':
			return set(internal_paras[0], rvalue, attributes, context);
		case '_':
		case T_NULL: // allow NULL to enable ignoring values
			return rvalue;
		case T_CLASSMEMBER: {
			attributes_t new_attributes = parse_attributes(internal_paras[0]);
			return set(internal_paras[1], rvalue, &new_attributes, context);
		}
		default:
			throw_MiscellaneousError("Invalid lvalue", this);
			break;
	}
	assert(false);
	return NULL;
}
ehretval_p EHI::eh_op_dot(ehretval_p *paras, ehcontext_t context) {
	ehretval_p base_var = eh_execute(paras[0], context);
	const char *accessor = paras[1]->get_stringval();
	if(base_var->type() == super_class_e) {
		return get_property(base_var->get_super_classval()->content(), accessor, context);
	} else {
		return get_property(base_var, accessor, context);
	}
}
ehretval_p EHI::eh_op_try(opnode_t *op, ehcontext_t context) {
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
		args = parent->make_tuple(new ehtuple_t(nargs, args_array));
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
		func = parent->make_binding(new ehbinding_t(obj, method));
	}
	return call_function(func, args, context);
}
ehretval_p EHI::call_function(ehretval_p function, ehretval_p args, ehcontext_t context) {
	// We special-case function calls on func_e and binding_e types; otherwise we'd end up in an infinite loop
	if(function->type() == binding_e || function->is_a(func_e)) {
		// This one time, we call a library method directly. If you want to override Function.operator_colon, too bad.
		return ehlm_Function_operator_colon(function, args, this);
	} else {
		return call_method(function, "operator()", args, context);
	}
}
ehretval_p EHInterpreter::make_method(ehlibmethod_t in) {
	ehobj_t *function_obj = new ehobj_t();
	ehretval_p func = this->make_object(function_obj);
	function_obj->parent = NULL;
	function_obj->type_id = func_e;
	ehfunc_t *f = new ehfunc_t(lib_e);
	f->libmethod_pointer = in;
	function_obj->object_data = ehretval_t::make_func(f);
	function_obj->inherit(this->function_object);
	return func;
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
		throw_VisibilityError(object, name, this);
	} else if(result->attribute.isstatic == static_e) {
		result->value = value;
		return result;
	} else {
		// set in this object
		ehmember_p new_member;
		new_member->attribute = result->attribute;
		new_member->value = value;
		obj->insert(name, new_member);
		return new_member;
	}
	return result;
}
// insert an ehmember_p directly on this object, without worrying about inheritance
ehmember_p EHI::set_member(ehretval_p object, const char *name, ehmember_p member, ehcontext_t context) {
	// caller should ensure object is actually an object
	ehobj_t *obj = object->get_objectval();
	// unbind bindings to myself
	ehretval_p value = member->value;
	if(value->type() == binding_e) {
		ehretval_p obj_data = value->get_bindingval()->object_data;
		if(obj_data->type() == object_e && obj == obj_data->get_objectval()) {
			member->value = value->get_bindingval()->method;
		}
	}
	if(obj->has(name)) {
		ehmember_p the_member = obj->get_known(name);
		if(the_member->attribute.isconst == const_e) {
			throw_ConstError(object, name, this);
		}
		if(the_member->attribute.visibility == private_e && !obj->context_compare(context)) {
			// pretend private members don't exist
			throw_VisibilityError(object, name, this);
		}
	}
	obj->insert(name, member);
	return member;
}
ehretval_p EHI::get_property(ehretval_p base_var, const char *name, ehcontext_t context) {
	ehretval_p object;
	if(base_var->is_object()) {
		object = base_var;
	} else {
		object = parent->get_primitive_class(base_var->type());
	}
	ehobj_t *obj = object->get_objectval();
	ehmember_p member = obj->inherited_get(name);
	if(member.null()) {
		throw_NameError(base_var, name, this);
	} else if (member->attribute.visibility == private_e && !obj->context_compare(context)) {
		throw_VisibilityError(base_var, name, this);
	}
	ehretval_p out = member->value;
	if(out->is_a(func_e)) {
		return parent->make_binding(new ehbinding_t(base_var, out));
	} else {
		return out;
	}
}
ehretval_p EHInterpreter::instantiate(ehretval_p obj) {
	ehobj_t *new_obj = new ehobj_t();
	ehretval_p ret = make_object(new_obj);
	ehretval_p to_instantiate;
	if(obj->type() == object_e) {
		to_instantiate = obj;
	} else {
		to_instantiate = get_primitive_class(obj->type());
	}
	ehobj_t *old_obj = to_instantiate->get_objectval();
	new_obj->type_id = old_obj->type_id;
	new_obj->parent = old_obj->parent;
	new_obj->inherit(to_instantiate);
	return ret;
}
ehretval_p EHInterpreter::resource_instantiate(int type_id, LibraryBaseClass *obj) {
	ehretval_p class_object = repo.get_object(type_id);
	ehretval_p obj_data = ehretval_t::make_resource(type_id, obj);

	ehretval_p ret = instantiate(class_object);
	ret->get_objectval()->object_data = obj_data;
	return ret;
}

/*
 * Arrays
 */
void EHI::array_insert(eharray_t *array, ehretval_p in, int place, ehcontext_t context) {
	/*
	 * We'll assume we're always getting a correct ehretval_p, referring to a
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
void EHInterpreter::eh_setarg(int argc, char **argv) {
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
ehretval_p EHInterpreter::get_command(const char *name) {
	if(this->cmdtable->has(name)) {
		return this->cmdtable->get(name);
	} else {
		return NULL;
	}
}
void EHInterpreter::insert_command(const char *name, ehretval_p cmd) {
	this->cmdtable->set(name, cmd);
}
void EHInterpreter::redirect_command(const char *redirect, const char *target) {
	ehretval_p targetcmd = get_command(target);
	assert(targetcmd != NULL);
	insert_command(redirect, targetcmd);
}
/*
 * Exceptions
 */
void EHI::handle_uncaught(eh_exception &e) {
	try {
		call_method(parent->global_object, "handleUncaught", e.content, parent->global_object);
	} catch(...) {
		std::cerr << "Exception occurred while handling uncaught exception" << std::endl;
	}
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

static attributes_t parse_attributes(ehretval_p node) {
	attributes_t attributes = attributes_t::make();
	for( ; node->get_opval()->nparas != 0; node = node->get_opval()->paras[1]) {
		switch(node->get_opval()->paras[0]->get_attributeval()) {
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
	return attributes;
}
