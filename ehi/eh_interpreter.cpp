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
#include "std_lib/Binding.hpp"
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
	{nullptr, nullptr}
};

#define LIBCMDREDENTRY(r, t) { #r, #t },
const char *libredirs[][2] = {
	LIBCMDREDENTRY(q, quit)
	{nullptr, nullptr}
};

static inline int count_nodes(const ehval_p node);
static attributes_t parse_attributes(ehval_p node);

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
EHInterpreter::EHInterpreter() : gc(), repo(), global_object(), function_object(), base_object(), optimize(false), included_files() {
	eh_init();
}
void EHInterpreter::eh_init(void) {
	ehobj_t *global_ehobj = new ehobj_t();
	global_object = Object::make(global_ehobj, this);

	// manually register GlobalObject in itself, run initializer
	global_ehobj->register_member_class<GlobalObject>(ehinit_GlobalObject, "GlobalObject", attributes_t::make_const(), this, global_object);

	// insert global command table
	ehval_p ret = Hash::make(this);
	this->cmdtable = ret->get<Hash>();
	ehmember_p command_table = ehmember_t::make(attributes_t::make_const(), ret);
	// insert command table into global objects
	global_object->get<Object>()->insert("commands", command_table);

	// fill command table
	for(int i = 0; libcmds[i].name != nullptr; i++) {
		ehval_p cmd = make_method(libcmds[i].cmd);
		insert_command(libcmds[i].name, cmd);
	}
	for(int i = 0; libredirs[i][0] != nullptr; i++) {
		redirect_command(libredirs[i][0], libredirs[i][1]);
	}

	gc.do_collect(global_object);
}
void EHInterpreter::eh_exit(void) {
	this->global_object->get<Object>()->members.erase("global");
	this->gc.do_collect(this->global_object);
	return;
}
EHInterpreter::~EHInterpreter() {
	eh_exit();
}

/*
 * Main execution function
 */
ehval_p EHI::eh_execute(ehval_p node, const ehcontext_t context) {
	// variables used
	ehval_p ret, operand1, operand2;
	bool b1, b2;

	if(node->is_a<Node>()) {
		//printf("Opcode %d: %d\n", node->opval->op, node->get<Node>()->nparas);
		Node::t *op = node->get<Node>();
		if(op == nullptr) {
			return nullptr;
		}
		ehval_p *paras = op->paras;
		switch(op->op) {
			case T_LITERAL:
				return paras[0];
			case T_NULL:
				return nullptr;
			case '_':
				throw_MiscellaneousError("Cannot use _ in expression", this);
				return nullptr;
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
				ret = eh_execute(paras[0], context);
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
				ehval_p obj = eh_execute(paras[0], context);
				ehval_p arg = eh_execute(paras[2], context);
				return call_method(obj, paras[1]->get<String>(), arg, context);
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
				if(!operand1->equal_type(operand2)) {
					throw_TypeError("Range members must have the same type", operand2, this);
				}
				return Range::make(operand1, operand2, parent);
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
				if(!this->toBool(operand1, context)->get<Bool>()) {
					return Bool::make(false);
				} else {
					operand2 = eh_execute(paras[1], context);
					return this->toBool(operand2, context);
				}
			case T_OR: // OR; use short-circuit operation
				operand1 = eh_execute(paras[0], context);
				if(this->toBool(operand1, context)->get<Bool>()) {
					return Bool::make(true);
				} else {
					operand2 = eh_execute(paras[1], context);
					return this->toBool(operand2, context);
				}
			case T_XOR:
				operand1 = eh_execute(paras[0], context);
				operand2 = eh_execute(paras[1], context);
				b1 = this->toBool(operand1, context)->get<Bool>();
				b2 = this->toBool(operand2, context)->get<Bool>();
				return Bool::make((b1 && !b2) || (!b1 && b2));
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
					paras[0]->get<String>(),
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
ehval_p EHI::eh_op_command(const char *name, ehval_p node, ehcontext_t context) {
	ehval_p value_r;
	// count for simple parameters
	int count = 0;
	// we're making an array of parameters
	ehval_p args = Array::make(parent);
	Array::t *paras = args->get<Array>();
	// loop through the paras given
	for( ; node->get<Node>()->nparas != 0; node = node->get<Node>()->paras[1]) {
		ehval_p node2 = node->get<Node>()->paras[0];
		// every para_expr should have an op associated with it
		assert(node2->is_a<Node>());
		ehval_p *node_paras = node2->get<Node>()->paras;
		switch(node2->get<Node>()->op) {
			case T_SHORTPARA: {
				// short paras: set each short-form option to the same thing
				if(node2->get<Node>()->nparas == 2) {
					// set to something else if specified
					value_r = eh_execute(node_paras[1], context);
				} else {
					// set to true by default
					value_r = Bool::make(true);
				}
				const char *shorts = node_paras[0]->get<String>();
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
				const char *index = node_paras[0]->get<String>();
				if(node2->get<Node>()->nparas == 1) {
					paras->string_indices[index] = Bool::make(true);
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
	paras->string_indices["_ehphp"] = Bool::make(true);
	// get the command to execute
	ehval_p libcmd = parent->get_command(name);
	ehval_p ret;
	if(libcmd != nullptr) {
		ret = this->call_function(libcmd, args, context);
	} else {
		ret = this->execute_cmd(name, paras);
	}
	// we're not returning anymore
	returning = false;
	return ret;
}
ehval_p EHI::eh_op_if(Node::t *op, ehcontext_t context) {
	if(this->toBool(eh_execute(op->paras[0], context), context)->get<Bool>()) {
		return eh_execute(op->paras[1], context);
	} else if(op->nparas == 2) {
		// if something { do_something() }
		return nullptr;
	} else {
		// loop through elsifs
		for(Node::t *iop = op->paras[2]->get<Node>(); iop->nparas != 0; iop = iop->paras[1]->get<Node>()) {
			ehval_p *current_block = iop->paras[0]->get<Node>()->paras;
			if(this->toBool(eh_execute(current_block[0], context), context)->get<Bool>()) {
				return eh_execute(current_block[1], context);
			}
		}
		// if there is a final else
		if(op->nparas == 4) {
			return eh_execute(op->paras[3], context);
		} else {
			return nullptr;
		}
	}
}
ehval_p EHI::eh_op_while(ehval_p *paras, ehcontext_t context) {
	ehval_p ret = nullptr;
	inloop++;
	breaking = 0;
	while(this->toBool(eh_execute(paras[0], context), context)->get<Bool>()) {
		ret = eh_execute(paras[1], context);
		LOOPCHECKS;
	}
	inloop--;
	return ret;
}
ehval_p EHI::eh_op_in(Node::t *op, ehcontext_t context) {
	ehval_p iteree_block, body_block;
	if(op->nparas == 3) {
		iteree_block = op->paras[1];
		body_block = op->paras[2];
	} else {
		iteree_block = op->paras[0];
		body_block = op->paras[1];
	}
	ehval_p iteree = eh_execute(iteree_block, context);
	ehval_p iterator = call_method(iteree, "getIterator", nullptr, context);
	inloop++;
	while(true) {
		ehval_p has_next = call_method_typed<Bool>(iterator, "hasNext", nullptr, context);
		if(!has_next->get<Bool>()) {
			break;
		}
		ehval_p next = call_method(iterator, "next", nullptr, context);
		if(op->nparas == 3) {
			attributes_t attributes = attributes_t::make(private_e, nonstatic_e, nonconst_e);
			set(op->paras[0], next, &attributes, context);
		}
		ehval_p ret = eh_execute(body_block, context);
		LOOPCHECKS;
	}
	inloop--;
	return iteree;
}
void EHI::eh_op_break(Node::t *op, ehcontext_t context) {
	ehval_p level_v = eh_execute(op->paras[0], context);
	if(!level_v->is_a<Integer>()) {
		throw_TypeError("break operator requires an Integer argument", level_v, this);
	}
	const int level = level_v->get<Integer>();
	// break as many levels as specified by the argument
	if(level > inloop) {
		throw_LoopError("break", level, this);
	}
	breaking = level;
}
void EHI::eh_op_continue(Node::t *op, ehcontext_t context) {
	ehval_p level_v = eh_execute(op->paras[0], context);
	if(!level_v->is_a<Integer>()) {
		throw_TypeError("continue operator requires an Integer argument", level_v, this);
	}
	const int level = level_v->get<Integer>();
	// break as many levels as specified by the argument
	if(level > inloop) {
		throw_LoopError("continue", level, this);
	}
	continuing = level;
}
ehval_p EHI::eh_op_array(ehval_p node, ehcontext_t context) {
	ehval_p ret = Array::make(parent);
	// need to count array members first, because they are reversed in our node.
	// That's not necessary with functions (where the situation is analogous), because the reversals that happen when parsing the prototype argument list and parsing the argument list in a call cancel each other out.
	int count = 0;
	for(ehval_p node2 = node; node2->get<Node>()->nparas != 0; node2 = node2->get<Node>()->paras[0]) {
		count++;
	}
	for(ehval_p node2 = node; node2->get<Node>()->nparas != 0; node2 = node2->get<Node>()->paras[0]) {
		array_insert(ret->get<Array>(), node2->get<Node>()->paras[1], --count, context);
	}
	return ret;
}
ehval_p EHI::eh_op_anonclass(ehval_p node, ehcontext_t context) {
	ehval_p ret = Hash::make(parent);
	Hash::ehhash_t *new_hash = ret->get<Hash>();
	for( ; node->get<Node>()->nparas != 0; node = node->get<Node>()->paras[0]) {
		ehval_p *myparas = node->get<Node>()->paras[1]->get<Node>()->paras;
		// nodes here will always have the name in para 0 and value in para 1
		ehval_p value = eh_execute(myparas[1], context);
		new_hash->set(myparas[0]->get<String>(), value);
	}
	return ret;
}
ehval_p EHI::eh_op_declareclosure(ehval_p *paras, ehcontext_t context) {
	Function::t *f = new Function::t(user_e);
	ehval_p ret = parent->instantiate(parent->repo.get_primitive_class<Function>());
	ehobj_t *function_object = ret->get<Object>();
	function_object->parent = context.scope;
	function_object->type_id = parent->function_object->get<Object>()->type_id;
	function_object->object_data = Function::make(f);
	f->code = paras[1];
	f->args = paras[0];
	return ret;
}
ehval_p EHI::eh_op_enum(Node::t *op, ehcontext_t context) {
	// unpack arguments
	const char *name = op->paras[0]->get<String>();
	ehval_p members_code = op->paras[1];
	ehval_p code = op->paras[2];

	// create Enum object
	ehval_p ret = Enum::t::make(name, this);
	Enum::t *e = Enum::t::extract_enum(ret);

	// extract enum members
	for(ehval_p node = members_code; ; node = node->get<Node>()->paras[0]) {
		ehval_p current_member;
		bool is_last;
		if(node->get<Node>()->op == ',') {
			current_member = node->get<Node>()->paras[1];
			is_last = false;
		} else {
			current_member = node;
			is_last = true;
		}

		// handle the member
		const char *member_name = current_member->get<Node>()->paras[0]->get<String>();
		if(current_member->get<Node>()->nparas == 1) {
			Enum::t::add_nullary_member(ret, member_name, this);
		} else {
			std::vector<std::string> params(0);
			for(ehval_p argument = current_member->get<Node>()->paras[1]; ; argument = argument->get<Node>()->paras[1]) {
				const char *name = argument->is_a<Node>() ? argument->get<Node>()->paras[0]->get<String>() : argument->get<String>();
				params.push_back(name);
				if(!argument->is_a<Node>() || argument->get<Node>()->op != ',') {
					break;
				}
			}
			Enum::t::add_member_with_arguments(ret, member_name, params, this);
		}

		if(is_last) {
			break;
		}
	}

	// execute internal code
	eh_execute(code, ehcontext_t(ret, e->contents));

	// remove inheritance (terrible hack)
	e->contents->get<Object>()->super.clear();

	// insert variable
	ehmember_p member;
	member->value = ret;
	context.scope->get<Object>()->insert(name, member);
	return ret;
}
ehval_p EHI::eh_op_declareclass(Node::t *op, ehcontext_t context) {
	// create the ehretval_t
	ehobj_t *new_obj = new ehobj_t();
	ehval_p ret = Object::make(new_obj, parent);

	// process parameters
	ehval_p code;
	const char *name = "(anonymous class)";
	if(op->nparas == 2) {
		// named class
		name = op->paras[0]->get<String>();
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
		context.scope->get<Object>()->insert(name, member);
#if 0
		// set class's own name property (commenting out until we can distinguish classes and prototypes better)
		ehmember_p name_member;
		name_member->value = String::make(strdup(name));
		new_obj->insert("name", name_member);
#endif
	}
	return ret;
}
ehval_p EHI::eh_op_tuple(ehval_p node, ehcontext_t context) {
	int nargs = 1;
	// first determine the size of the tuple
	for(ehval_p tmp = node;
		tmp->is_a<Node>() && tmp->get<Node>()->op == ',' && tmp->get<Node>()->nparas != 0;
		tmp = tmp->get<Node>()->paras[1], nargs++
	) {}
	ehretval_a new_args(nargs);

	ehval_p arg_node = node;
	// now, fill the output tuple
	for(int i = 0; i < nargs; i++) {
		Node::t *op = arg_node->get<Node>();
		if(op->op == ',') {
			new_args[i] = eh_execute(op->paras[0], context);
			arg_node = arg_node->get<Node>()->paras[1];
		} else {
			new_args[i] = eh_execute(arg_node, context);
			assert(i == nargs - 1);
			break;
		}
	}
	return Tuple::make(nargs, new_args, parent);
}
ehval_p EHI::eh_op_classmember(Node::t *op, ehcontext_t context) {
	// parse the attributes into an attributes_t
	attributes_t attributes = parse_attributes(op->paras[0]);
	// set the member
	return set(op->paras[1], nullptr, &attributes, context);
}
ehval_p EHI::eh_op_switch(ehval_p *paras, ehcontext_t context) {
	ehval_p ret;
	// because we use continue, we'll pretend this is a loop
	inloop++;

	// switch variable
	ehval_p switchvar = eh_execute(paras[0], context);
	for(ehval_p node = paras[1]; node->get<Node>()->nparas != 0; node = node->get<Node>()->paras[1]) {
		Node::t *op = node->get<Node>()->paras[0]->get<Node>();
		// execute default
		if(op->nparas == 1) {
			ret = eh_execute(op->paras[0], context);
		} else {
			ehval_p casevar = eh_execute(op->paras[0], context);
			ehval_p decider;
			// try to call function
			if(casevar->deep_is_a<Function>() || casevar->is_a<Binding>()) {
				decider = call_function(casevar, switchvar, context);
				if(!decider->is_a<Bool>()) {
					throw_TypeError("Method in a switch case must return a Bool", decider, this);
				}
			} else {
				decider = call_method_typed<Bool>(casevar, "operator==", switchvar, context);
			}
			// apply the decider
			if(decider->get<Bool>()) {
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
	return nullptr;
}
ehval_p EHI::eh_op_given(ehval_p *paras, ehcontext_t context) {
	// switch variable
	ehval_p switchvar = eh_execute(paras[0], context);
	for(ehval_p node = paras[1]; node->get<Node>()->nparas != 0; node = node->get<Node>()->paras[1]) {
		const Node::t *op = node->get<Node>()->paras[0]->get<Node>();
		// execute default
		if(op->nparas == 1) {
			return eh_execute(op->paras[0], context);
		}
		ehval_p casevar = eh_execute(op->paras[0], context);
		ehval_p decider;
		if(casevar->deep_is_a<Function>() || casevar->is_a<Binding>()) {
			decider = call_function(casevar, switchvar, context);
			if(!decider->is_a<Bool>()) {
				throw_TypeError("Method in a switch case must return a Bool", decider, this);
			}
		} else {
			decider = call_method(switchvar, "operator==", casevar, context);
			if(!decider->is_a<Bool>()) {
				throw_TypeError("operator== does not return a bool", decider, this);
			}
		}
		if(decider->get<Bool>()) {
			return eh_execute(op->paras[1], context);
		}
	}
	throw_MiscellaneousError("No matching case in given statement", this);
	return nullptr;
}

bool EHI::match(ehval_p node, ehval_p var, ehcontext_t context) {
	Node::t *op = node->get<Node>();
	switch(op->op) {
		case '_':
			return true;
		case '@': {
			const char *name = op->paras[0]->get<String>();
			ehmember_p member = ehmember_t::make(attributes_t::make_private(), var);
			context.scope->set_member(name, member, context, this);
			return true;
		}
		case '|': {
			return match(op->paras[0], var, context) || match(op->paras[1], var, context);
		}
		case ',': {
			if(!var->is_a<Tuple>()) {
				return false;
			}
			Tuple::t *t = var->get<Tuple>();
			const int size = t->size();
			int i = 0;
			for(ehval_p arg_node = node; ; arg_node = arg_node->get<Node>()->paras[1], i++) {
				if(i == size) {
					return false;
				}
				Node::t *op = arg_node->get<Node>();
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
			ehval_p member = eh_execute(op->paras[0], context);
			ehval_p member_em = member->data();
			if(!member_em->is_a<Enum_Member>()) {
				throw_TypeError("match case is not an Enum.Member", member, this);
			}
			Enum_Member::t *em = member_em->get<Enum_Member>();

			var = var->data();
			if(!var->is_a<Enum_Instance>()) {
				return false;
			}
			Enum_Instance::t *var_ei = var->get<Enum_Instance>();

			if(member_em->naive_compare(var_ei->member()) != 0) {
				return false;
			}
			int size = em->size;
			if(op->paras[1]->get<Node>()->op != '(') {
				throw_MiscellaneousError("Invalid argument in Enum.Member match", this);
			}
			int nargs = 1;
			ehval_p args = op->paras[1]->get<Node>()->paras[0];
			for(ehval_p tmp = args;
				tmp->is_a<Node>() && tmp->get<Node>()->op == ',' && tmp->get<Node>()->nparas != 0;
				tmp = tmp->get<Node>()->paras[1], nargs++
			);
			if(nargs != size) {
				throw_MiscellaneousError("Invalid argument number in Enum.Member match", this);
			}
			ehval_p arg_node = args;
			for(int i = 0; i < nargs; i++) {
				Node::t *op = arg_node->get<Node>();
				if(op->op == ',') {
					if(!match(op->paras[0], var_ei->get(i), context)) {
						return false;
					}
					arg_node = arg_node->get<Node>()->paras[1];
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
			ehval_p casevar = eh_execute(node, context);
			ehval_p decider = call_method_typed<Bool>(var, "operator==", casevar, context);
			return decider->get<Bool>();
		}
	}
}

ehval_p EHI::eh_op_match(ehval_p *paras, ehcontext_t context) {
	// switch variable
	ehval_p switchvar = eh_execute(paras[0], context);
	for(ehval_p node = paras[1]; node->get<Node>()->nparas != 0; node = node->get<Node>()->paras[1]) {
		ehval_p case_node = node->get<Node>()->paras[0];
		if(match(case_node->get<Node>()->paras[0], switchvar, context)) {
			return eh_execute(case_node->get<Node>()->paras[1], context);
		}
	}
	throw_MiscellaneousError("No matching case in match statement", this);
	return nullptr;
}
ehval_p EHI::eh_op_customop(ehval_p *paras, ehcontext_t context) {
	ehval_p lhs = eh_execute(paras[0], context);
	ehval_p rhs = eh_execute(paras[2], context);
	std::string op = paras[1]->get<String>();
	return call_method(lhs, ("operator" + op).c_str(), rhs, context);
}
ehval_p EHI::eh_op_colon(ehval_p *paras, ehcontext_t context) {
	// parse arguments
	ehval_p function = eh_execute(paras[0], context);
	ehval_p args = eh_execute(paras[1], context);
	return call_function(function, args, context);
}
ehval_p EHI::eh_op_dollar(ehval_p node, ehcontext_t context) {
	ehmember_p var = context.scope->get<Object>()->get_recursive(node->get<String>(), context);
	if(var == nullptr) {
		throw_NameError(context.scope, node->get<String>(), this);
		return nullptr;
	} else {
		return var->value;
	}
}
ehval_p EHI::eh_op_set(ehval_p *paras, ehcontext_t context) {
	ehval_p rvalue = eh_execute(paras[1], context);
	return set(paras[0], rvalue, nullptr, context);
}
ehval_p EHI::set(ehval_p lvalue, ehval_p rvalue, attributes_t *attributes, ehcontext_t context) {
	ehval_p *internal_paras = lvalue->get<Node>()->paras;
	switch(lvalue->get<Node>()->op) {
		case T_ARROW: {
			ehval_p args[2];
			args[0] = eh_execute(internal_paras[1], context);
			args[1] = rvalue;
			ehval_p base_var = eh_execute(internal_paras[0], context);
			return call_method(base_var, "operator->=", Tuple::make(2, args, parent), context);
		}
		case '.': {
			ehval_p base_var = eh_execute(internal_paras[0], context);
			if(base_var->is_a<SuperClass>()) {
				throw_TypeError("Cannot set member on parent class", base_var, this);
			}
			// This is hard, since we will, for once, need to modify in-place. For now, only support objects. Functions too, just for fun.
			if(!base_var->is_a<Object>()) {
				throw_TypeError("Cannot set member on primitive", base_var, this);
			}
			// accessor is guaranteed to be a string
			const char *accessor = internal_paras[1]->get<String>();
			if(attributes == nullptr) {
				base_var->set_property(accessor, rvalue, context, this);
			} else {
				ehmember_p new_member = ehmember_t::make(*attributes, rvalue);
				base_var->set_member(accessor, new_member, context, this);
			}
			return rvalue;
		}
		case '$': {
			const char *name = internal_paras[0]->get<String>();
			attributes_t attributes_container = attributes_t::make();
			if(attributes == nullptr) {
				ehmember_p member = context.scope->get<Object>()->get_recursive(name, context);
				if(member != nullptr) {
					if(member->isconst()) {
						// bug: if the const member is actually in a higher scope, this error message will be wrong
						throw_ConstError(context.scope, name, this);
					}
					member->value = rvalue;
					return rvalue;
				}
				attributes = &attributes_container;
			}
			ehmember_p new_member = ehmember_t::make(*attributes, rvalue);
			context.scope->set_member(name, new_member, context, this);
			return rvalue;
		}
		case ',': {
			ehval_p arg_node = lvalue;
			for(int i = 0; true; i++) {
				Node::t *op = arg_node->get<Node>();
				ehval_p internal_rvalue;
				if(!rvalue->is_a<Null>()) {
					internal_rvalue = call_method(rvalue, "operator->", Integer::make(i), context);
				} else {
					internal_rvalue = nullptr;
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
			// ignore rvalue
			return rvalue;
		case T_NULL:
			// assert rvalue is null, and ignore it
			if(!rvalue->is_a<Null>()) {
				throw_MiscellaneousError("Non-null value assigned to null", this);
			}
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
	return nullptr;
}
ehval_p EHI::eh_op_dot(ehval_p *paras, ehcontext_t context) {
	ehval_p base_var = eh_execute(paras[0], context);
	const char *accessor = paras[1]->get<String>();
	if(base_var->is_a<SuperClass>()) {
		return base_var->get<SuperClass>()->get_property(accessor, context, this);
	} else {
		return base_var->get_property(accessor, context, this);
	}
}
ehval_p EHI::eh_op_try(Node::t *op, ehcontext_t context) {
	ehval_p try_block = op->paras[0];
	ehval_p catch_blocks = op->paras[1];
	if(op->nparas == 2) {
		return eh_try_catch(try_block, catch_blocks, context);
	} else {
		ehval_p finally_block = op->paras[2];
		ehval_p ret;
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
ehval_p EHI::eh_try_catch(ehval_p try_block, ehval_p catch_blocks, ehcontext_t context) {
	Node::t *catch_op = catch_blocks->get<Node>();
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
		context.scope->set_member("exception", exception_member, context, this);
		for(; catch_op->nparas != 0; catch_op = catch_op->paras[1]->get<Node>()) {
			Node::t *catch_block = catch_op->paras[0]->get<Node>();
			if(catch_block->nparas == 1) {
				return eh_execute(catch_block->paras[0], context);
			} else {
				// conditional catch
				ehval_p decider = toBool(eh_execute(catch_block->paras[0], context), context);
				if(decider->get<Bool>()) {
					return eh_execute(catch_block->paras[1], context);
				}
			}
		}
		// re-throw if we couldn't catch it
		throw;
	}
}
ehval_p EHI::eh_always_execute(ehval_p code, ehcontext_t context) {
	// Execute even if we're breaking or continuing or whatever
	bool old_returning = returning;
	returning = false;
	int old_breaking = breaking;
	breaking = 0;
	int old_continuing = continuing;
	continuing = 0;
	ehval_p ret = eh_execute(code, context);
	continuing = old_continuing;
	breaking = old_breaking;
	returning = old_returning;
	return ret;
}
// Perform an arbitrary operation defined as a method taking a single argument
ehval_p EHI::perform_op(const char *name, int nargs, ehval_p *paras, ehcontext_t context) {
	ehval_p base_var = eh_execute(paras[0], context);
	ehval_p args = nullptr;
	if(nargs == 1) {
		args = eh_execute(paras[1], context);
	} else if(nargs > 1) {
		ehretval_a args_array(nargs);
		for(int i = 0; i < nargs; i++) {
			args_array[i] = eh_execute(paras[i + 1], context);
		}
		args = Tuple::make(nargs, args_array, parent);
	}
	return call_method(base_var, name, args, context);
}
/*
 * Functions
 */
ehval_p EHI::call_method(ehval_p obj, const char *name, ehval_p args, ehcontext_t context) {
	ehval_p func = obj->get_property_no_binding(name, context, this);
	if(func->deep_is_a<Function>()) {
		return Function::exec(obj, func, args, this);
	} else {
		return call_method(func, "operator()", args, context);
	}
}
ehval_p EHI::call_function(ehval_p function, ehval_p args, ehcontext_t context) {
	// We special-case function calls on Function and Binding types; otherwise we'd end up in an infinite loop
	if(function->is_a<Binding>()) {
		return ehlm_Binding_operator_colon(function, args, this);
	} else if(function->deep_is_a<Function>()) {
		// This one time, we call a library method directly. If you want to override Function.operator_colon, too bad.
		return ehlm_Function_operator_colon(function, args, this);
	} else {
		return call_method(function, "operator()", args, context);
	}
}
ehval_p EHInterpreter::make_method(ehlibmethod_t in) {
	ehobj_t *function_obj = new ehobj_t();
	ehval_p func = Object::make(function_obj, this);
	function_obj->parent = nullptr;
	function_obj->type_id = function_object->get<Object>()->type_id;
	Function::t *f = new Function::t(lib_e);
	f->libmethod_pointer = in;
	function_obj->object_data = Function::make(f);
	function_obj->inherit(this->function_object);
	return func;
}
/*
 * Classes
 */
ehval_p EHInterpreter::instantiate(ehval_p obj) {
	ehobj_t *new_obj = new ehobj_t();
	ehval_p ret = Object::make(new_obj, this);
	ehval_p to_instantiate;
	if(obj->is_a<Object>()) {
		to_instantiate = obj;
	} else {
		to_instantiate = repo.get_object(obj);
	}
	ehobj_t *old_obj = to_instantiate->get<Object>();
	new_obj->type_id = old_obj->type_id;
	new_obj->parent = old_obj->parent;
	new_obj->inherit(to_instantiate);
	return ret;
}
ehval_p EHInterpreter::resource_instantiate(int type_id, ehval_p obj) {
	ehval_p class_object = repo.get_object(type_id);

	ehval_p ret = instantiate(class_object);
	ret->get<Object>()->object_data = obj;
	return ret;
}

/*
 * Arrays
 */
void EHI::array_insert(Array::t *array, ehval_p in, int place, ehcontext_t context) {
	/*
	 * We'll assume we're always getting a correct ehval_p, referring to a
	 * T_ARRAYMEMBER token. If there is 1 parameter, that means it's a
	 * non-labeled array member, which we'll give an integer array index; if
	 * there are 2, we'll either use the integer array index or a hash of the
	 * string index.
	 */
	if(in->get<Node>()->nparas == 1) {
		// if there is no explicit key, simply use the place argument
		array->int_indices[place] = eh_execute(in->get<Node>()->paras[0], context);
	} else {
		const ehval_p label = eh_execute(in->get<Node>()->paras[0], context);
		ehval_p var = eh_execute(in->get<Node>()->paras[1], context);
		if(label->is_a<Integer>()) {
			array->int_indices[label->get<Integer>()] = var;
		} else if(label->is_a<String>()) {
			array->string_indices[label->get<String>()] = var;
		} else {
			throw_TypeError_Array_key(label, this);
		}
	}
}
/*
 * Command line arguments
 */
void EHInterpreter::eh_setarg(int argc, char **argv) {
	// insert argc
	ehmember_p argc_v;
	// argc - 1, because argv[0] is ehi itself
	argc_v->value = Integer::make(argc - 1);
	global_object->get<Object>()->insert("argc", argc_v);

	// insert argv
	ehmember_p argv_v;
	argv_v->value = Array::make(this);

	// all members of argv are strings
	for(int i = 1; i < argc; i++) {
		argv_v->value->get<Array>()->int_indices[i - 1] = String::make(strdup(argv[i]));
	}
	global_object->get<Object>()->insert("argv", argv_v);
}
/*
 * Commands
 */
ehval_p EHInterpreter::get_command(const char *name) {
	if(this->cmdtable->has(name)) {
		return this->cmdtable->get(name);
	} else {
		return nullptr;
	}
}
void EHInterpreter::insert_command(const char *name, ehval_p cmd) {
	this->cmdtable->set(name, cmd);
}
void EHInterpreter::redirect_command(const char *redirect, const char *target) {
	ehval_p targetcmd = get_command(target);
	assert(targetcmd != nullptr);
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
static inline int count_nodes(const ehval_p node) {
	// count a list like an argument list. Assumes correct layout.
	int i = 0;
	for(ehval_p tmp = node;
		tmp->get<Node>()->nparas != 0;
		tmp = tmp->get<Node>()->paras[0], i++
	) {}
	return i;
}

static attributes_t parse_attributes(ehval_p node) {
	attributes_t attributes = attributes_t::make();
	for( ; node->get<Node>()->nparas != 0; node = node->get<Node>()->paras[1]) {
		switch(node->get<Node>()->paras[0]->get<Attribute>()) {
			case Attribute::publica_e:
				attributes.visibility = public_e;
				break;
			case Attribute::privatea_e:
				attributes.visibility = private_e;
				break;
			case Attribute::statica_e:
				attributes.isstatic = static_e;
				break;
			case Attribute::consta_e:
				attributes.isconst = const_e;
				break;
		}
	}
	return attributes;
}
