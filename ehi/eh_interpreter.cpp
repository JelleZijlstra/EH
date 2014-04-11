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

#include "std_lib/Array.hpp"
#include "std_lib/Attribute.hpp"
#include "std_lib/Binding.hpp"
#include "std_lib/ConstError.hpp"
#include "std_lib/Enum.hpp"
#include "std_lib/Function.hpp"
#include "std_lib/Generator.hpp"
#include "std_lib/GlobalObject.hpp"
#include "std_lib/LoopError.hpp"
#include "std_lib/Map.hpp"
#include "std_lib/MiscellaneousError.hpp"
#include "std_lib/NameError.hpp"
#include "std_lib/Node.hpp"
#include "std_lib/Object.hpp"
#include "std_lib/Range.hpp"
#include "std_lib/Exception.hpp"
#include "std_lib/Tuple.hpp"
#include "std_lib/VisibilityError.hpp"

// Needs to come after inclusion of Attribute
#include "eh.bison.hpp"

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
	// turn off GC during initialization
	gc.stop_collecting();
	// global object is an Object, initially without a Class
	ehobj_t *global_ehobj = new ehobj_t();
	global_object = Object::make(global_ehobj, this);

	// manually register GlobalObject in itself, run initializer
	unsigned int type_id = global_ehobj->register_member_class("GlobalObject", ehinit_GlobalObject, attributes_t::make_const(), this);
	global_ehobj->cls = repo.get_object(type_id);
	ehinstance_init_GlobalObject(global_ehobj, this);

	// insert global command table
	ehval_p ret = Hash::make(this);
	this->cmdtable = ret->get<Hash>();
	ehmember_p command_table = ehmember_t::make(attributes_t::make_const(), ret);
	// insert command table into global objects
	global_object->get<Object>()->insert("commands", command_table);

	// fill command table
	for(int i = 0; libcmds[i].name != nullptr; i++) {
		ehval_p cmd = Function::make(new Function::t(libcmds[i].cmd), this);
		insert_command(libcmds[i].name, cmd);
	}
	for(int i = 0; libredirs[i][0] != nullptr; i++) {
		redirect_command(libredirs[i][0], libredirs[i][1]);
	}
	gc.resume_collecting();
	gc.do_collect();
}
void EHInterpreter::eh_exit(void) {
	this->global_object->get<Object>()->members.erase("global");
	this->gc.do_collect();
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

	if(Node::is_a(node)) {
		Enum_Instance::t *op = node->get<Enum_Instance>();
		if(op == nullptr) {
			return nullptr;
		}
		ehval_p *paras = op->members;
		switch(op->member_id) {
			case T_LITERAL:
				return paras[0];
			case T_NULL:
				return nullptr;
			case T_ANYTHING:
				throw_MiscellaneousError("Cannot use _ in expression", this);
		/*
		 * Unary operators
		 */
			case T_MATCH_SET:
				throw_MiscellaneousError("Cannot use @ outside of match statement", this);
			case T_BINARY_COMPLEMENT: // bitwise negation
			  return perform_op("operator~", 0, paras, context);
			case T_NOT: // Boolean not
			  return perform_op("operator!", 0, paras, context);
		/*
		 * Control flow
		 */
			case T_IF:
				return eh_op_if(T_IF, paras, context);
			case T_IF_ELSE:
				return eh_op_if(T_IF_ELSE, paras, context);
			case T_WHILE:
				return eh_op_while(paras, context);
			case T_FOR:
				return eh_op_for(paras, context);
			case T_FOR_IN:
				return eh_op_for_in(paras, context);
			case T_MATCH:
				return eh_op_match(paras, context);
		/*
		 * Exceptions
		 */
			case T_TRY:
				return eh_op_try(paras, context);
			case T_TRY_FINALLY:
				return eh_op_try_finally(paras, context);
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
			case T_YIELD:
				return eh_op_yield(paras[0], context);
			case T_BREAK: // break out of a loop
				eh_op_break(paras, context);
				break;
			case T_CONTINUE: // continue in a loop
				eh_op_continue(paras, context);
				break;
		/*
		 * Object access
		 */
			case T_CALL: // function call
				return eh_op_colon(paras, context);
			case T_CALL_METHOD: {
				ehval_p obj = eh_execute(paras[0], context);
				ehval_p arg = eh_execute(paras[2], context);
				return call_method(obj, paras[1]->get<String>(), arg, context);
			}
			case T_BAR: {
				ehval_p obj = eh_execute(paras[0], context);
				ehval_p arg = eh_execute(paras[1], context);
				return call_method(obj, "operator|", arg, context);
			}
			case T_RAW: {
				ehretval_a args(2);
				if(Node::is_a(paras[0]) && paras[0]->get<Enum_Instance>()->member_id == T_GROUPING) {
					args[0] = paras[0]->get<Enum_Instance>()->members[0];
				} else {
					args[0] = paras[0];
				}
				args[1] = Node_Context::make(context, parent);
				return Tuple::make(2, args, parent);
			}
			case T_THIS: // direct access to the context object
				return context.object;
			case T_SCOPE:
				return context.scope;
		/*
		 * Object definitions
		 */
			case T_FUNC: // function definition
				return eh_op_declareclosure(paras, context, false);
			case T_GENERATOR:
				return eh_op_declareclosure(paras, context, true);
			case T_NAMED_CLASS: // named class declaration
				return eh_op_named_class(paras, context);
			case T_CLASS: // anonymous class declaration
				return eh_op_class(paras, context);
			case T_CLASS_MEMBER:
				return eh_op_classmember(paras, context);
			case T_ENUM:
				return eh_op_enum(paras, context);
			case T_ARRAY_LITERAL: // array declaration
				return eh_op_array(paras[0], context);
			case T_HASH_LITERAL: // hash
				return eh_op_anonclass(paras[0], context);
			case T_COMMA: // tuple
				return eh_op_tuple(node, context);
			case T_RANGE:
				operand1 = eh_execute(paras[0], context);
				operand2 = eh_execute(paras[1], context);
				if(!operand1->equal_type(operand2)) {
					throw_TypeError("Range members must have the same type", operand2, this);
				}
				return Range::make(operand1, operand2, parent);
			case T_MIXED_TUPLE:
				return eh_op_mixed_tuple(node, context);
			case T_NAMED_ARGUMENT:
				throw_MiscellaneousError("Cannot use T_NAMED_ARGUMENT in expression", this);
			case T_AS:
				throw_MiscellaneousError("Cannot use T_AS in expression", this);
		/*
		 * Binary operators
		 */
			case T_ACCESS:
				return eh_op_dot(paras, context);
			case T_INSTANCE_ACCESS:
				return eh_op_instance_access(paras, context);
			case T_ARROW:
				return perform_op("operator->", 1, paras, context);
			case T_GROUPING:
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
				return Bool::make(b1 != b2);
		/*
		 * Variable manipulation
		 */
			case T_ASSIGN:
				return eh_op_set(paras, context);
			case T_VARIABLE: // variable dereference
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
				std::cerr << "Unexpected opcode " << op->member_id;
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
ehval_p EHI::eh_op_command(const char *name, ehval_p node, const ehcontext_t &context) {
	ehval_p value_r;
	// count for simple parameters
	int count = 0;
	// we're making an array of parameters
	ehval_p args = Map::make(this);
	Map::t *paras = args->get<Map>();
	// loop through the paras given
	for( ; node->get<Enum_Instance>()->member_id != T_END; node = node->get<Enum_Instance>()->members[1]) {
		ehval_p node2 = node->get<Enum_Instance>()->members[0];
		// every para_expr should have an op associated with it
		if(Node::is_a(node2)) {
			ehval_p *node_paras = node2->get<Enum_Instance>()->members;
			switch(node2->get<Enum_Instance>()->member_id) {
				case T_SHORTPARA: {
					// short paras: set each short-form option to the same thing
					value_r = eh_execute(node_paras[1], context);
					const char *shorts = node_paras[0]->get<String>();
					for(size_t i = 0, len = strlen(shorts); i < len; i++) {
						char index[2];
						index[0] = shorts[i];
						index[1] = '\0';
						paras->set(String::make(strdup(index)), value_r);
					}
					break;
				}
				case T_LONGPARA:
					// long-form paras
					paras->set(node_paras[0], eh_execute(node_paras[1], context));
					break;
				default: // non-named parameters with an expression
					paras->set(Integer::make(count), eh_execute(node2, context));
					count++;
					break;
			}
		} else {
			paras->set(Integer::make(count), eh_execute(node2, context));
			count++;
		}
	}
	// insert indicator that this is an EH-PHP command
	paras->set(String::make(strdup("_ehphp")), Bool::make(true));
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
ehval_p EHI::eh_op_if(int token, ehval_p *paras, const ehcontext_t &context) {
	if(this->toBool(eh_execute(paras[0], context), context)->get<Bool>()) {
		return eh_execute(paras[1], context);
	} else {
		// loop through elsifs
		for(Enum_Instance::t *iop = paras[2]->get<Enum_Instance>(); iop->member_id != T_END; iop = iop->members[1]->get<Enum_Instance>()) {
			ehval_p *current_block = iop->members[0]->get<Enum_Instance>()->members;
			if(this->toBool(eh_execute(current_block[0], context), context)->get<Bool>()) {
				return eh_execute(current_block[1], context);
			}
		}
		// if there is a final else
		if(token == T_IF_ELSE) {
			return eh_execute(paras[3], context);
		} else {
			return nullptr;
		}
	}
}
ehval_p EHI::eh_op_while(ehval_p *paras, const ehcontext_t &context) {
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
ehval_p EHI::do_for_loop(ehval_p iteree_block, ehval_p body_block, int op, ehval_p set_block, const ehcontext_t &context) {
	ehval_p iteree = eh_execute(iteree_block, context);
	ehval_p iterator = call_method(iteree, "getIterator", nullptr, context);
	inloop++;
	while(true) {
		ehval_p next;
		try {
			next = call_method(iterator, "next", nullptr, context);
		} catch(eh_exception &e) {
            const unsigned int type_id = e.content->get_type_id(get_parent());
            if(type_id == get_parent()->type_ids.EmptyIterator) {
            	break;
            } else {
            	throw;
            }
		}
		if(op == T_FOR_IN) {
			attributes_t attributes = attributes_t(private_e, nonstatic_e, nonconst_e);
			set(set_block, next, &attributes, context);
		}
		ehval_p ret = eh_execute(body_block, context);
		LOOPCHECKS;
	}
	inloop--;
	return iteree;
}
ehval_p EHI::eh_op_for(ehval_p *paras, const ehcontext_t &context) {
	return do_for_loop(paras[0], paras[1], T_FOR, nullptr, context);
}
ehval_p EHI::eh_op_for_in(ehval_p *paras, const ehcontext_t &context) {
	return do_for_loop(paras[1], paras[2], T_FOR_IN, paras[0], context);
}
ehval_p EHI::eh_op_yield(ehval_p para, const ehcontext_t &context) {
	throw_RuntimeError("yield is not allowed in the AST interpreter", this);
}
void EHI::eh_op_break(ehval_p *paras, const ehcontext_t &context) {
	ehval_p level_v = eh_execute(paras[0], context);
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
void EHI::eh_op_continue(ehval_p *paras, const ehcontext_t &context) {
	ehval_p level_v = eh_execute(paras[0], context);
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
ehval_p EHI::eh_op_array(ehval_p node, const ehcontext_t &context) {
	// need to count array members first, because they are reversed in our node.
	// That's not necessary with functions (where the situation is analogous), because the reversals that happen when parsing the prototype argument list and parsing the argument list in a call cancel each other out.
	int count = 0;
	for(ehval_p node2 = node; node2->get<Enum_Instance>()->member_id != T_END; node2 = node2->get<Enum_Instance>()->members[1]) {
		count++;
	}
	ehval_p ret = Array::make(parent, count);
	for(ehval_p node2 = node; node2->get<Enum_Instance>()->member_id != T_END; node2 = node2->get<Enum_Instance>()->members[1]) {
		ret->get<Array>()->insert(--count, eh_execute(node2->get<Enum_Instance>()->members[0], context));
	}
	return ret;
}
ehval_p EHI::eh_op_anonclass(ehval_p node, const ehcontext_t &context) {
	ehval_p ret = Hash::make(parent);
	Hash::ehhash_t *new_hash = ret->get<Hash>();
	for( ; node->get<Enum_Instance>()->member_id != T_END; node = node->get<Enum_Instance>()->members[1]) {
		ehval_p *myparas = node->get<Enum_Instance>()->members[0]->get<Enum_Instance>()->members;
		// nodes here will always have the name in para 0 and value in para 1
		ehval_p value = eh_execute(myparas[1], context);
		new_hash->set(myparas[0]->get<String>(), value);
	}
	return ret;
}
ehval_p EHI::eh_op_declareclosure(ehval_p *paras, const ehcontext_t &context, bool is_generator) {
	Function::t *f = new Function::t(Function::user_e);
	f->code = paras[1];
	f->args = paras[0];
	f->parent = context.scope;
	f->is_generator = is_generator;
	return Function::make(f, parent);
}
ehval_p EHI::eh_op_enum(ehval_p *paras, const ehcontext_t &context) {
	// unpack arguments
	const char *name = paras[0]->get<String>();
	ehval_p members_code = paras[1];
	ehval_p code = paras[2];

	ehval_p ret = Enum::make_enum_class(name, context.scope, parent);
	Enum::t *enum_obj = ret->get<Enum>();

	// extract enum members
	for(ehval_p node = members_code; ; node = node->get<Enum_Instance>()->members[1]) {
		ehval_p current_member;
		bool is_last;
		if(node->get<Enum_Instance>()->member_id == T_COMMA) {
			current_member = node->get<Enum_Instance>()->members[0];
			is_last = false;
		} else {
			current_member = node;
			is_last = true;
		}

		// handle the member
		const char *member_name = current_member->get<Enum_Instance>()->members[0]->get<String>();
		std::vector<std::string> params(0);
		if(current_member->get<Enum_Instance>()->member_id == T_ENUM_WITH_ARGUMENTS) {
			for(ehval_p argument = current_member->get<Enum_Instance>()->members[1]; ; argument = argument->get<Enum_Instance>()->members[1]) {
				const char *param_name = Node::is_a(argument) ? argument->get<Enum_Instance>()->members[0]->get<String>() : argument->get<String>();
				params.push_back(param_name);
				if(!Node::is_a(argument) || argument->get<Enum_Instance>()->member_id != T_COMMA) {
					break;
				}
			}
		}
		enum_obj->add_enum_member(member_name, params, parent);

		if(is_last) {
			break;
		}
	}

	// execute internal code
	eh_execute(code, ret);

	// insert variable
	ehmember_p member = ehmember_t::make(attributes_t(), ret);
	context.scope->set_member(name, member, context, this);
	return ret;
}
ehval_p EHI::eh_op_class(ehval_p *paras, const ehcontext_t &context) {
	return declare_class("(anonymous class)", paras[0], context);
}
ehval_p EHI::eh_op_named_class(ehval_p *paras, const ehcontext_t &context) {
	const char *name = paras[0]->get<String>();
	ehval_p ret = declare_class(name, paras[1], context);
	ehmember_p member = ehmember_t::make(attributes_t(), ret);
	context.scope->set_member(name, member, context, this);
#if 0
	// set class's own name property (commenting out until we can distinguish classes and prototypes better)
	ehmember_p name_member = ehmember_t::make(attributes_t(), String::make(strdup(name)));
	new_obj->insert("name", name_member);
#endif
	return ret;
}
ehval_p EHI::declare_class(const char *name, ehval_p code, const ehcontext_t &context) {
	// create the class object
	ehclass_t *new_obj = new ehclass_t(name);
	ehval_p ret = Class::make(new_obj, parent);

	new_obj->type_id = parent->repo.register_class(name, ret);
	new_obj->parent = context.scope;

	// execute the code within the class
	eh_execute(code, ret);

	return ret;
}
ehval_p EHI::eh_op_mixed_tuple(ehval_p node, const ehcontext_t &context) {
	int nargs = 0;
	for(ehval_p tmp = node; ; tmp = tmp->get<Enum_Instance>()->members[1]) {
		if(Node::is_a(tmp) && tmp->get<Enum_Instance>()->member_id == T_MIXED_TUPLE) {
			// continue with next
			auto op = tmp->get<Enum_Instance>();
			// increment nargs unless it is a T_NAMED_ARGUMENT
			if(!(Node::is_a(op->members[0]) && op->members[0]->get<Enum_Instance>()->member_id == T_NAMED_ARGUMENT)) {
				nargs++;
			}
		} else {
			// increment again if last member is not a T_NAMED_ARGUMENT
			if(!(Node::is_a(tmp) && tmp->get<Enum_Instance>()->member_id == T_NAMED_ARGUMENT)) {
				nargs++;
			}
			break;
		}
	}
	auto twsk = new Tuple_WithStringKeys::t(nargs);
	for(int i = 0; ; node = node->get<Enum_Instance>()->members[1]) {
		bool is_last = !(Node::is_a(node) && node->get<Enum_Instance>()->member_id == T_MIXED_TUPLE);
		ehval_p current_node = is_last ? node : ehval_p(node->get<Enum_Instance>()->members[0]);
		if(Node::is_a(current_node) && current_node->get<Enum_Instance>()->member_id == T_NAMED_ARGUMENT) {
			auto op = current_node->get<Enum_Instance>();
			twsk->set(op->members[0]->get<String>(), eh_execute(op->members[1], context));
		} else {
			twsk->set(i, eh_execute(current_node, context));
			i++;
		}
		if(is_last) {
			break;
		}
	}

	return Tuple_WithStringKeys::make(twsk, parent);
}
ehval_p EHI::eh_op_tuple(ehval_p node, const ehcontext_t &context) {
	unsigned int nargs = 1;
	// first determine the size of the tuple
	for(ehval_p tmp = node;
		Node::is_a(tmp) && tmp->get<Enum_Instance>()->member_id == T_COMMA;
		tmp = tmp->get<Enum_Instance>()->members[1], nargs++
	) {}
	ehretval_a new_args(nargs);

	ehval_p arg_node = node;
	// now, fill the output tuple
	for(unsigned int i = 0; i < nargs; i++) {
		if(Node::is_a(arg_node) && arg_node->get<Enum_Instance>()->member_id == T_COMMA) {
			new_args[i] = eh_execute(arg_node->get<Enum_Instance>()->members[0], context);
			arg_node = arg_node->get<Enum_Instance>()->members[1];
		} else {
			new_args[i] = eh_execute(arg_node, context);
			assert(i == nargs - 1);
			break;
		}
	}
	return Tuple::make(nargs, new_args, parent);
}
ehval_p EHI::eh_op_classmember(ehval_p *paras, const ehcontext_t &context) {
	// parse the attributes into an attributes_t
	attributes_t attributes = parse_attributes(paras[0]);
	// set the member
	return set(paras[1], nullptr, &attributes, context);
}

bool EHI::match(ehval_p node, ehval_p var, const ehcontext_t &context) {
	if(!Node::is_a(node)) {
		ehval_p casevar = eh_execute(node, context);
		ehval_p decider = call_method_typed<Bool>(var, "operator==", casevar, context);
		return decider->get<Bool>();
	} else {
		Enum_Instance::t *op = node->get<Enum_Instance>();
		switch(op->member_id) {
			case T_ANYTHING:
				return true;
			case T_MATCH_SET: {
				const char *name = op->members[0]->get<String>();
				ehmember_p member = ehmember_t::make(attributes_t::make_private(), var);
				context.scope->set_member(name, member, context, this);
				return true;
			}
			case T_AS: {
				if(match(op->members[0], var, context)) {
					const char *name = op->members[1]->get<String>();
					ehmember_p member = ehmember_t::make(attributes_t::make_private(), var);
					context.scope->set_member(name, member, context, this);
					return true;
				} else {
					return false;
				}
			}
			case T_BAR: {
				return match(op->members[0], var, context) || match(op->members[1], var, context);
			}
			case T_COMMA: {
				if(!var->is_a<Tuple>()) {
					return false;
				}
				Tuple::t *t = var->get<Tuple>();
				const int size = t->size();
				int i = 0;
				for(ehval_p arg_node = node; ; arg_node = arg_node->get<Enum_Instance>()->members[1], i++) {
					// size does not match (pattern is too long)
					if(i == size) {
						return false;
					}
					Enum_Instance::t *inner_op = arg_node->get<Enum_Instance>();
					if(inner_op->member_id == T_COMMA) {
						if(!match(inner_op->members[0], t->get(i), context)) {
							return false;
						}
					} else {
						// size does not match (pattern is too short)
						if(i != size - 1) {
							return false;
						}
						return match(arg_node, t->get(i), context);
					}
				}
			}
			case T_CALL: {
				ehval_p member = eh_execute(op->members[0], context);
				if(!member->is_a<Enum_Instance>()) {
					throw_TypeError("match case is not an Enum.Member", member, this);
				}
				const auto em = member->get<Enum_Instance>();
				if(em->members != nullptr) {
					throw_MiscellaneousError("Invalid argument in Enum.Member match", this);
				}

				if(!var->is_a<Enum_Instance>()) {
					return false;
				}
				const auto var_ei = var->get<Enum_Instance>();

				if(var_ei->members == nullptr || em->type_compare(var_ei) != 0) {
					return false;
				}
				const unsigned int size = em->nmembers;
				if(op->members[1]->get<Enum_Instance>()->member_id != T_GROUPING) {
					throw_MiscellaneousError("Invalid argument in Enum.Member match", this);
				}
				unsigned int nargs = 1;
				ehval_p args = op->members[1]->get<Enum_Instance>()->members[0];
				for(ehval_p tmp = args;
					Node::is_a(tmp) && tmp->get<Enum_Instance>()->member_id == T_COMMA && tmp->get<Enum_Instance>()->member_id != T_END;
					tmp = tmp->get<Enum_Instance>()->members[1], nargs++
				);
				if(nargs != size) {
					throw_MiscellaneousError("Invalid argument number in Enum.Member match", this);
				}
				ehval_p arg_node = args;
				for(unsigned int i = 0; i < nargs; i++) {
					if(Node::is_a(arg_node) && arg_node->get<Enum_Instance>()->member_id == T_COMMA) {
						ehval_p *paras = arg_node->get<Enum_Instance>()->members;
						if(!match(paras[0], var_ei->get(i), context)) {
							return false;
						}
						arg_node = paras[1];
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
			case T_GROUPING: {
				return match(op->members[0], var, context);
			}
			default: {
				ehval_p casevar = eh_execute(node, context);
				ehval_p decider = call_method_typed<Bool>(casevar, "operator==", var, context);
				return decider->get<Bool>();
			}
		}
	}
}

ehval_p EHI::eh_op_match(ehval_p *paras, const ehcontext_t &context) {
	// switch variable
	ehval_p switchvar = eh_execute(paras[0], context);
	for(ehval_p node = paras[1]; node->get<Enum_Instance>()->member_id != T_END; node = node->get<Enum_Instance>()->members[1]) {
		Enum_Instance::t *case_node = node->get<Enum_Instance>()->members[0]->get<Enum_Instance>();
		if(case_node->member_id == T_WHEN) {
			bool does_match = match(case_node->members[0], switchvar, context);
			if(does_match && this->toBool(eh_execute(case_node->members[1], context), context)->get<Bool>()) {
				return eh_execute(case_node->members[2], context);
			}
		} else if(match(case_node->members[0], switchvar, context)) {
			return eh_execute(case_node->members[1], context);
		}
	}
	std::string switchvar_str = toString(switchvar, context)->get<String>();
	const char *msg = strdup(("No matching case in match statement: " + switchvar_str).c_str());
	throw_MiscellaneousError(msg, this);
}
ehval_p EHI::eh_op_customop(ehval_p *paras, const ehcontext_t &context) {
	ehval_p lhs = eh_execute(paras[0], context);
	ehval_p rhs = eh_execute(paras[2], context);
	std::string op = paras[1]->get<String>();
	return call_method(lhs, ("operator" + op).c_str(), rhs, context);
}
ehval_p EHI::eh_op_colon(ehval_p *paras, const ehcontext_t &context) {
	// parse arguments
	ehval_p function = eh_execute(paras[0], context);
	ehval_p args = eh_execute(paras[1], context);
	return call_function(function, args, context);
}
ehval_p EHI::eh_op_dollar(ehval_p node, const ehcontext_t &context) {
	ehmember_p var = context.scope->get_property_up_scope_chain(node->get<String>(), context, this);
	if(var.null()) {
		throw_NameError(context.scope, node->get<String>(), this);
	} else {
		return var->value;
	}
}
ehval_p EHI::eh_op_set(ehval_p *paras, const ehcontext_t &context) {
	ehval_p lvalue = paras[0];
	/*
	 * Allow syntactic sugar for function definitions.
	 * Placing this here is slightly hackish, but a solution placing it in
	 * set() would entail evaluating the rvalue in set() itself (which
	 * seems problematic when set() is called during a function instantiation)
	 * and would allow weird syntax like foo x, bar = x * x, 3.
	 */
	auto ei = lvalue->get<Enum_Instance>();
	if(ei->member_id == T_CALL) {
		ehval_p rvalue = Node::make(eh_addnode(T_FUNC, ei->members[1], paras[1]), parent);
		ehval_p op_paras[2] = {ei->members[0], rvalue};
		return eh_op_set(op_paras, context);
	} else if(ei->member_id == T_CLASS_MEMBER) {
		auto inner_ei = ei->members[1]->get<Enum_Instance>();
		if(inner_ei->member_id == T_CALL) {
			ehval_p new_lvalue = Node::make(eh_addnode(T_CLASS_MEMBER, ei->members[0], inner_ei->members[0]), parent);
			ehval_p new_rvalue = Node::make(eh_addnode(T_FUNC, inner_ei->members[1], paras[1]), parent);
			ehval_p op_paras[2] = {new_lvalue, new_rvalue};
			return eh_op_set(op_paras, context);
		}
	}
	ehval_p rvalue = eh_execute(paras[1], context);
	return set(paras[0], rvalue, nullptr, context);
}
ehval_p EHI::set(ehval_p lvalue, ehval_p rvalue, attributes_t *attributes, const ehcontext_t &context) {
	ehval_p *internal_paras = lvalue->get<Enum_Instance>()->members;
	switch(lvalue->get<Enum_Instance>()->member_id) {
		case T_ARROW: {
			ehval_p base_var = eh_execute(internal_paras[0], context);
			ehval_p args[2];
			args[0] = eh_execute(internal_paras[1], context);
			args[1] = rvalue;
			return call_method(base_var, "operator->=", Tuple::make(2, args, parent), context);
		}
		case T_INSTANCE_ACCESS: {
			ehval_p base_var = eh_execute(internal_paras[0], context);
			const char *accessor = internal_paras[1]->get<String>();
			if(attributes == nullptr) {
				base_var->set_instance_property(accessor, rvalue, context, this);
			} else {
				ehmember_p new_member = ehmember_t::make(*attributes, rvalue);
				base_var->set_instance_member(accessor, new_member, context, this);
			}
			return rvalue;
		}
		case T_ACCESS: {
			ehval_p base_var = eh_execute(internal_paras[0], context);
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
		case T_VARIABLE: {
			const char *name = internal_paras[0]->get<String>();
			return set_bare_variable(name, rvalue, context, attributes);
		}
		case T_COMMA: {
			ehval_p arg_node = lvalue;
			for(int i = 0; true; i++) {
				Enum_Instance::t *op = arg_node->get<Enum_Instance>();
				ehval_p internal_rvalue;
				if(!rvalue->is_a<Null>()) {
					internal_rvalue = call_method(rvalue, "operator->", Integer::make(i), context);
				} else {
					internal_rvalue = nullptr;
				}
				if(op->member_id == T_COMMA) {
					set(op->members[0], internal_rvalue, attributes, context);
					arg_node = op->members[1];
				} else {
					set(arg_node, internal_rvalue, attributes, context);
					break;
				}
			}
			return rvalue;
		}
		case T_MIXED_TUPLE: {
			ehval_p arg_node = lvalue;
			for(int i = 0; true; ) {
				auto op = arg_node->get<Enum_Instance>();
				ehval_p lvalue_node = (op->member_id == T_MIXED_TUPLE) ? ehval_p(op->members[0]) : arg_node;
				ehval_p internal_rvalue;
				if(rvalue->is_a<Null>()) {
					// it's null, do nothing
				} else if(Node::is_a(lvalue_node) && lvalue_node->get<Enum_Instance>()->member_id == T_NAMED_ARGUMENT) {
					auto inner_op = lvalue_node->get<Enum_Instance>();
					ehval_p name = inner_op->members[0];

					ehval_p named_arg_rvalue;
					if(call_method_typed<Bool>(rvalue, "has", name, context)->get<Bool>()) {
						named_arg_rvalue = call_method(rvalue, "operator->", name, context);
					} else {
						named_arg_rvalue = eh_execute(inner_op->members[1], context);
					}
					attributes_t attrs = (attributes == nullptr) ? attributes_t::make_private() : *attributes;
					context.scope->set_member(name->get<String>(), ehmember_p(attrs, named_arg_rvalue), context, this);
				} else {
					internal_rvalue = call_method(rvalue, "operator->", Integer::make(i), context);
					i++;
					set(lvalue_node, internal_rvalue, attributes, context);
				}
				if(op->member_id != T_MIXED_TUPLE) {
					break;
				} else {
					arg_node = op->members[1];
				}
			}
			return rvalue;
		}
		case T_GROUPING:
			return set(internal_paras[0], rvalue, attributes, context);
		case T_ANYTHING:
			// ignore rvalue
			return rvalue;
		case T_NULL:
			// assert rvalue is null, and ignore it
			if(!rvalue->is_a<Null>()) {
				throw_MiscellaneousError("Non-null value assigned to null", this);
			}
			return rvalue;
		case T_CLASS_MEMBER: {
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
ehval_p EHI::eh_op_dot(ehval_p *paras, const ehcontext_t &context) {
	ehval_p base_var = eh_execute(paras[0], context);
	const char *accessor = paras[1]->get<String>();
	return base_var->get_property(accessor, context, this);
}
ehval_p EHI::eh_op_instance_access(ehval_p *paras, const ehcontext_t &context) {
	ehval_p base_var = eh_execute(paras[0], context);
	const char *accessor = paras[1]->get<String>();
	return base_var->get_instance_member_throwing(accessor, context, this);
}
ehval_p EHI::eh_op_try_finally(ehval_p *paras, const ehcontext_t &context) {
	ehval_p try_block = paras[0];
	ehval_p catch_blocks = paras[1];
	ehval_p finally_block = paras[2];
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
ehval_p EHI::eh_op_try(ehval_p *paras, const ehcontext_t &context) {
	ehval_p try_block = paras[0];
	ehval_p catch_blocks = paras[1];
	return eh_try_catch(try_block, catch_blocks, context);
}
ehval_p EHI::eh_try_catch(ehval_p try_block, ehval_p catch_blocks, const ehcontext_t &context) {
	Enum_Instance::t *catch_op = catch_blocks->get<Enum_Instance>();
	// don't try/catch if there are no catch blocks
	if(catch_op->member_id == T_END) {
		return eh_execute(try_block, context);
	}
	try {
		return eh_execute(try_block, context);
	} catch(eh_exception &e) {
		// insert exception into current scope
		attributes_t attributes = attributes_t(public_e, nonstatic_e, nonconst_e);
		ehmember_p exception_member = ehmember_t::make(attributes, e.content);
		context.scope->set_member("exception", exception_member, context, this);
		for(; catch_op->member_id != T_END; catch_op = catch_op->members[1]->get<Enum_Instance>()) {
			Enum_Instance::t *catch_block = catch_op->members[0]->get<Enum_Instance>();
			if(catch_block->member_id == T_CATCH) {
				return eh_execute(catch_block->members[0], context);
			} else {
				// conditional catch (T_CATCH_IF)
				ehval_p decider = toBool(eh_execute(catch_block->members[0], context), context);
				if(decider->get<Bool>()) {
					return eh_execute(catch_block->members[1], context);
				}
			}
		}
		// re-throw if we couldn't catch it
		throw;
	}
}
ehval_p EHI::eh_always_execute(ehval_p code, const ehcontext_t &context) {
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
ehval_p EHI::perform_op(const char *name, unsigned int nargs, ehval_p *paras, const ehcontext_t &context) {
	ehval_p base_var = eh_execute(paras[0], context);
	ehval_p args = nullptr;
	if(nargs == 1) {
		args = eh_execute(paras[1], context);
	} else if(nargs > 1) {
		ehretval_a args_array(nargs);
		for(unsigned int i = 0; i < nargs; i++) {
			args_array[i] = eh_execute(paras[i + 1], context);
		}
		args = Tuple::make(nargs, args_array, parent);
	}
	return call_method(base_var, name, args, context);
}
/*
 * Functions
 */
ehval_p EHI::call_method(ehval_p obj, const char *name, ehval_p args, const ehcontext_t &context) {
	ehval_p func = obj->get_property_no_binding(name, context, this)->value;
	if(func->is_a<Function>()) {
		return Function::exec(obj, func, args, this);
	} else {
		return call_method(func, "operator()", args, context);
	}
}
ehval_p EHI::call_function(ehval_p function, ehval_p args, const ehcontext_t &context) {
	// We special-case function calls on Function and Binding types; otherwise we'd end up in an infinite loop
	if(function->is_a<Binding>()) {
		// This one time, we call a library method directly. If you want to override Binding.operator_colon, too bad.
		return ehlm_Binding_operator_colon(function, args, this);
	} else if(function->is_a<Function>()) {
		// Avoid calling Function.operator(), so that we can preserve the object
		return Function::exec(context.object, function, args, this);
	} else {
		return call_method(function, "operator()", args, context);
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
	argv_v->value = Array::make(this, argc - 1);

	// all members of argv are strings
	for(int i = 1; i < argc; i++) {
		argv_v->value->get<Array>()->v[i - 1] = String::make(strdup(argv[i]));
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
		tmp->get<Enum_Instance>()->member_id != T_END;
		tmp = tmp->get<Enum_Instance>()->members[0], i++
	) {}
	return i;
}

static attributes_t parse_attributes(ehval_p node) {
	attributes_t attributes = attributes_t();
	for( ; node->get<Enum_Instance>()->member_id != T_END; node = node->get<Enum_Instance>()->members[1]) {
		switch(node->get<Enum_Instance>()->members[0]->get<Enum_Instance>()->member_id) {
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
