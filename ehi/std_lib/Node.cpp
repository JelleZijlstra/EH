/*
 * Node
 * Represent a node of the AST.
 */
#include <sstream>

#include "std_lib_includes.hpp"
#include "Node.hpp"
#include "ArgumentError.hpp"
#include "Attribute.hpp"
#include "../eh.bison.hpp"

#define TOKEN(name, nparas) cls->add_enum_member(#name, vec ## nparas, parent, name)

/*
 * The following tokens are not listed here, since they do not appear in the generated AST:
T_INTEGER, T_FLOAT, T_BOOL, T_AS, T_IN, T_FINALLY, T_STRING, T_RIGHTSHIFTEQ, T_LEFTSHIFTEQ, T_BINXOREQ, T_BINOREQ, T_BINANDEQ, T_XOREQ, T_OREQ, T_ANDEQ, T_MODULOEQ, T_DIVIDEEQ, T_MULTIPLYEQ, T_MINEQ, T_PLUSEQ, T_MINMIN, T_PLUSPLUS, T_NEGATIVE
 */

const std::vector<std::string> vec0 = {};
const std::vector<std::string> vec1 = {"parameter"};
const std::vector<std::string> vec2 = {"parameter", "parameter"};
const std::vector<std::string> vec3 = {"parameter", "parameter", "parameter"};
const std::vector<std::string> vec4 = {"parameter", "parameter", "parameter", "parameter"};

EH_ENUM_INITIALIZER(Node) {
	REGISTER_METHOD(Node, execute);
	REGISTER_METHOD(Node, decompile);
	TOKEN(T_IF, 3);
	TOKEN(T_ELSE, 2);
	TOKEN(T_ELSIF, 2);
	TOKEN(T_WHILE, 2);
	TOKEN(T_FOR, 2);
	TOKEN(T_GIVEN, 2);
	TOKEN(T_MATCH, 2);
	TOKEN(T_END, 0);
	TOKEN(T_SWITCH, 2);
	TOKEN(T_DEFAULT, 1);
	TOKEN(T_CASE, 2);
	TOKEN(T_BREAK, 1);
	TOKEN(T_CONTINUE, 1);
	TOKEN(T_FUNC, 2);
	TOKEN(T_RET, 1);
	TOKEN(T_SEPARATOR, 2);
	TOKEN(T_NULL, 0);
	TOKEN(T_ENUM, 3);
	TOKEN(T_CLASS, 1);
	TOKEN(T_CLASS_MEMBER, 2);
	TOKEN(T_LITERAL, 1);
	TOKEN(T_TRY, 2);
	TOKEN(T_CATCH, 1);
	TOKEN(T_ATTRIBUTE, 2);
	TOKEN(T_ARRAY_MEMBER, 2);
	TOKEN(T_DOUBLEARROW, 2);
	TOKEN(T_CALL_METHOD, 3);
	TOKEN(T_TRY_FINALLY, 3);
	TOKEN(T_CATCH_IF, 2);
	TOKEN(T_FOR_IN, 3);
	TOKEN(T_NAMED_CLASS, 2);
	TOKEN(T_IF_ELSE, 4);
	TOKEN(T_NULLARY_ENUM, 1);
	TOKEN(T_ENUM_WITH_ARGUMENTS, 2);
	TOKEN(T_ARRAY_MEMBER_NO_KEY, 1);
	TOKEN(T_ANYTHING, 0);
	TOKEN(T_GROUPING, 1);
	TOKEN(T_ASSIGN, 2);
	TOKEN(T_BINARY_COMPLEMENT, 1);
	TOKEN(T_NOT, 1);
	TOKEN(T_MATCH_SET, 1);
	TOKEN(T_COMMA, 2);
	TOKEN(T_ARRAY_LITERAL, 1);
	TOKEN(T_HASH_LITERAL, 1);
	TOKEN(T_CALL, 2);
	TOKEN(T_ACCESS, 2);
	TOKEN(T_COMMAND, 2);
	TOKEN(T_SHORTPARA, 2);
	TOKEN(T_LONGPARA, 2);
	TOKEN(T_VARIABLE, 1);
	TOKEN(T_XOR, 2);
	TOKEN(T_OR, 2);
	TOKEN(T_AND, 2);
	TOKEN(T_BAR, 2);
	TOKEN(T_ARROW, 2);
	TOKEN(T_RANGE, 2);
	TOKEN(T_SCOPE, 0);
	TOKEN(T_THIS, 0);
	TOKEN(T_RAW, 1);
	TOKEN(T_NAMED_ARGUMENT, 2);
	TOKEN(T_MIXED_TUPLE, 2);
	TOKEN(T_AS, 2);
	TOKEN(T_WHEN, 3);
	TOKEN(T_LIST, 1); // used for the convenience of the compiler: gives a tuple of code blocks rather than the nasty T_COMMA stuff
	REGISTER_CLASS(Node, Context);
}

EH_METHOD(Node, execute) {
	ASSERT_OBJ_TYPE(Enum_Instance, "Node.execute");
	ASSERT_TYPE(args, Node_Context, "Node.execute");
	return ehi->eh_execute(obj, *args->get<Node_Context>());
}

EH_METHOD(Node, decompile) {
	ASSERT_OBJ_TYPE(Enum_Instance, "Node.decompile");
	auto str = obj->decompile(0);
	return String::make(strdup(str.c_str()));
}

static void add_end(std::ostringstream &out, int levels) {
	out << "\n";
	add_tabs(out, levels);
	out << "end";
}
static void decompile_try_catch(std::ostringstream &out, ehval_p *paras, int level) {
	out << "try\n";
	add_tabs(out, level + 1);
	out << paras[0]->decompile(level + 1);

	Enum_Instance::t *catch_op = paras[1]->get<Enum_Instance>();
	for(; catch_op->member_id != T_END; catch_op = catch_op->members[1]->get<Enum_Instance>()) {
		Enum_Instance::t *catch_block = catch_op->members[0]->get<Enum_Instance>();
		out << "\n";
		add_tabs(out, level);
		if(catch_block->member_id == T_CATCH) {
			out << "catch\n";
			add_tabs(out, level + 1);
			out << catch_block->members[0]->decompile(level + 1);
		} else {
			// conditional catch (T_CATCH_IF)
			out << "catch if " << catch_block->members[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << catch_block->members[1]->decompile(level + 1);
		}
	}
}
static void decompile_match_like(std::ostringstream &out, const char *name, ehval_p *paras, int level) {
	out << name << " " << paras[0]->decompile(level);
	for(ehval_p node = paras[1]; node->get<Enum_Instance>()->member_id != T_END; node = node->get<Enum_Instance>()->members[1]) {
		out << "\n";
		add_tabs(out, level + 1);
		Enum_Instance::t *inner_op = node->get<Enum_Instance>()->members[0]->get<Enum_Instance>();
		if(inner_op->member_id == T_DEFAULT) {
			out << "default\n";
			add_tabs(out, level + 2);
			out << inner_op->members[0]->decompile(level + 2);
		} else {
			out << "case " << inner_op->members[0]->decompile(level + 1) << "\n";
			add_tabs(out, level + 2);
			out << inner_op->members[1]->decompile(level + 2);
		}
	}
	add_end(out, level);
}
static void decompile_if(std::ostringstream &out, ehval_p *paras, int level) {
	out << "if " << paras[0]->decompile(level) << "\n";
	add_tabs(out, level + 1);
	out << paras[1]->decompile(level + 1);
	for(Enum_Instance::t *iop = paras[2]->get<Enum_Instance>(); iop->member_id != T_END; iop = iop->members[1]->get<Enum_Instance>()) {
		ehval_p *current_block = iop->members[0]->get<Enum_Instance>()->members;
		out << "\n";
		add_tabs(out, level);
		out << "elsif " << current_block[0]->decompile(level) << "\n";
		add_tabs(out, level + 1);
		out << current_block[1]->decompile(level);
	}
}

std::string Node::decompile(int level) const {
	std::ostringstream out;
	const Node *op = this;
	if(op == nullptr) {
		return "";
	}
	switch(op->member_id) {
		case T_LITERAL:
			out << op->members[0]->decompile(level);
			break;
		case T_NULL:
			out << "()";
			break;
		case T_ANYTHING:
			out << "_";
			break;
		case T_ASSIGN:
			out << op->members[0]->decompile(level) << " = " << op->members[1]->decompile(level);
			break;
		case T_ADD:
			out << op->members[0]->decompile(level) << " + " << op->members[1]->decompile(level);
			break;
		case T_SUBTRACT:
			out << op->members[0]->decompile(level) << " - " << op->members[1]->decompile(level);
			break;
		case T_MULTIPLY:
			out << op->members[0]->decompile(level) << " * " << op->members[1]->decompile(level);
			break;
		case T_DIVIDE:
			out << op->members[0]->decompile(level) << " / " << op->members[1]->decompile(level);
			break;
		case T_MODULO:
			out << op->members[0]->decompile(level) << " % " << op->members[1]->decompile(level);
			break;
		case T_GREATER:
			out << op->members[0]->decompile(level) << " > " << op->members[1]->decompile(level);
			break;
		case T_BAR:
			out << op->members[0]->decompile(level) << " | " << op->members[1]->decompile(level);
			break;
		case T_COMMA:
		case T_MIXED_TUPLE:
			out << op->members[0]->decompile(level) << ", " << op->members[1]->decompile(level);
			break;
		case T_ACCESS:
			out << op->members[0]->decompile(level) << "." << op->members[1]->get<String>();
			break;
		case T_CALL_METHOD:
			out << op->members[0]->decompile(level) << "." << op->members[1]->get<String>() << "(" << op->members[2]->decompile(level) << ")";
			break;
		case T_CALL:
			out << op->members[0]->decompile(level) << " " << op->members[1]->decompile(level);
			break;
		case T_NOT:
			out << "!" << op->members[0]->decompile(level);
			break;
		case T_BINARY_COMPLEMENT:
			out << "~" << op->members[0]->decompile(level);
			break;
		case T_NAMED_ARGUMENT:
			out << op->members[0]->get<String>() << ": " << op->members[1]->decompile(level);
			break;
		case T_VARIABLE:
			out << op->members[0]->get<String>();
			break;
		case T_GROUPING:
			out << '(' << op->members[0]->decompile(level) << ')';
			break;
		case T_RANGE:
			out << op->members[0]->decompile(level) << " .. " << op->members[1]->decompile(level);
			break;
		case T_ARROW:
			// no space
			out << op->members[0]->decompile(level) << "->" << op->members[1]->decompile(level);
			break;
		case T_AND:
			out << op->members[0]->decompile(level) << " && " << op->members[1]->decompile(level);
			break;
		case T_OR:
			out << op->members[0]->decompile(level) << " || " << op->members[1]->decompile(level);
			break;
		case T_XOR:
			out << op->members[0]->decompile(level) << " ^^ " << op->members[1]->decompile(level);
			break;
		case T_SEPARATOR: {
			out << op->members[0]->decompile(level);
			ehval_p r = op->members[1];
			if(!Node::is_a(r) || r->get<Enum_Instance>()->member_id != T_SEPARATOR || r->get<Enum_Instance>()->member_id != T_END) {
				out << "\n";
				add_tabs(out, level);
				out << op->members[1]->decompile(level);
			}
			break;
		}
		case T_END:
			// ignore, used to end lists
			break;
		case T_THIS:
			out << "this";
			break;
		case T_SCOPE:
			out << "scope";
			break;
		case T_RET:
			out << "ret " << op->members[0]->decompile(level);
			break;
		case T_CLASS_MEMBER:
			out << op->members[0]->decompile(level) << op->members[1]->decompile(level);
			break;
		case T_ATTRIBUTE:
			out << op->members[0]->decompile(level) << " " << op->members[1]->decompile(level);
			break;
		case T_IF:
			decompile_if(out, op->members, level);
			add_end(out, level);
			break;
		case T_IF_ELSE:
			decompile_if(out, op->members, level);
			out << "\n";
			add_tabs(out, level);
			out << "else\n";
			add_tabs(out, level + 1);
			out << op->members[3]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_WHILE:
			out << "while " << op->members[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->members[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_TRY:
			decompile_try_catch(out, op->members, level);
			add_end(out, level);
			break;
		case T_TRY_FINALLY:
			decompile_try_catch(out, op->members, level);
			out << "\n";
			add_tabs(out, level);
			out << "finally\n";
			add_tabs(out, level + 1);
			out << op->members[2]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_NAMED_CLASS:
			out << "class " << op->members[0]->get<String>() << "\n";
			add_tabs(out, level + 1);
			out << op->members[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_ENUM:
			out << "enum " << op->members[0]->get<String>() << "\n";
			add_tabs(out, level + 1);
			// decompile member list
			for(ehval_p node = op->members[1]; ; node = node->get<Enum_Instance>()->members[1]) {
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
				out << current_member->get<Enum_Instance>()->members[0]->get<String>();
				if(current_member->get<Enum_Instance>()->member_id == T_ENUM_WITH_ARGUMENTS) {
					out << "(";
					for(ehval_p argument = current_member->get<Enum_Instance>()->members[1]; ; argument = argument->get<Enum_Instance>()->members[1]) {
						ehval_p name = Node::is_a(argument) ? ehval_p(argument->get<Enum_Instance>()->members[0]) : argument;
						out << name->get<String>();
						if(!Node::is_a(argument) || argument->get<Enum_Instance>()->member_id != T_COMMA) {
							break;
						} else {
							out << ", ";
						}
					}
					out << ")";
				}

				if(is_last) {
					break;
				} else {
					out << ", ";
				}
			}
			out << "\n";
			add_tabs(out, level + 1);
			// decompile code
			out << op->members[2]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_CLASS:
			out << "class\n";
			add_tabs(out, level + 1);
			out << op->members[0]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_FUNC:
			out << "func: " << op->members[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->members[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_SWITCH:
			decompile_match_like(out, "switch", op->members, level);
			break;
		case T_MATCH:
			decompile_match_like(out, "match", op->members, level);
			break;
		case T_GIVEN:
			decompile_match_like(out, "given", op->members, level);
			break;
		case T_FOR:
			out << "for " << op->members[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->members[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_FOR_IN:
			out << "for " << op->members[0]->decompile(level) << " in " << op->members[1]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->members[2]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_ARRAY_LITERAL:
			out << "[";
			for(ehval_p n = op->members[0]; n->get<Enum_Instance>()->member_id != T_END; n = n->get<Enum_Instance>()->members[1]) {
				Enum_Instance::t *member_op = n->get<Enum_Instance>()->members[0]->get<Enum_Instance>();
				out << member_op->members[0]->decompile(level);
				if(member_op->member_id == T_ARRAY_MEMBER) {
					out << " => " << member_op->members[1]->decompile(level);
				}
				if(n->get<Enum_Instance>()->members[1]->get<Enum_Instance>()->member_id != T_END) {
					out << ", ";
				}
			}
			out << "]";
			break;
		case T_MATCH_SET:
			out << "@";
			out << op->members[0]->get<String>();
			break;
		case T_HASH_LITERAL:
			out << "{";
			for(ehval_p n = op->members[0]; n->get<Enum_Instance>()->member_id != T_END; n = n->get<Enum_Instance>()->members[1]) {
				Enum_Instance::t *member_op = n->get<Enum_Instance>()->members[0]->get<Enum_Instance>();
				out << member_op->members[0]->decompile(level);
				out << ": " << member_op->members[1]->decompile(level);
				if(n->get<Enum_Instance>()->members[1]->get<Enum_Instance>()->member_id != T_END) {
					out << ", ";
				}
			}
			out << "}";
			break;
		case T_COMMAND:
			out << "$" << op->members[0]->get<String>();
			for(ehval_p node = op->members[1]; node->get<Enum_Instance>()->member_id != T_END; node = node->get<Enum_Instance>()->members[1]) {
				Enum_Instance::t *node2 = node->get<Enum_Instance>()->members[0]->get<Enum_Instance>();
				switch(node2->member_id) {
					case T_SHORTPARA:
						out << " -" << node2->members[0]->decompile(level);
						out << "=" << node2->members[1]->decompile(level);
						break;
					case T_LONGPARA:
						out << " --" << node2->members[0]->decompile(level);
						out << "=" << node2->members[1]->decompile(level);
						break;
					default:
						out << " " << node->get<Enum_Instance>()->members[0]->decompile(level);
						break;
				}
			}
			out << "\n";
			break;
		default:
			out << "(cannot decode value: " << op->member_id << ")";
			break;
	}
	return out.str();
}

Node *eh_addnode(unsigned int opcode) {
	return new Node(opcode, 0);
}
Node *eh_addnode(unsigned int opcode, ehval_p first) {
	Node *op = new Node(opcode, 1);
	op->members[0] = first;
	return op;
}
Node *eh_addnode(unsigned int opcode, ehval_p first, ehval_p second) {
	Node *op = new Node(opcode, 2);
	op->members[0] = first;
	op->members[1] = second;
	return op;
}
Node *eh_addnode(unsigned int opcode, ehval_p first, ehval_p second, ehval_p third) {
	Node *op = new Node(opcode, 3);
	op->members[0] = first;
	op->members[1] = second;
	op->members[2] = third;
	return op;
}
Node *eh_addnode(unsigned int opcode, ehval_p first, ehval_p second, ehval_p third, ehval_p fourth) {
	Node *op = new Node(opcode, 4);
	op->members[0] = first;
	op->members[1] = second;
	op->members[2] = third;
	op->members[3] = fourth;
	return op;
}

EH_INITIALIZER(Node_Context) {
	REGISTER_METHOD(Node_Context, operator_colon);
	REGISTER_METHOD(Node_Context, getObject);
	REGISTER_METHOD(Node_Context, getScope);
}

void Node_Context::printvar(printvar_set &set, int level, class EHI *ehi) {
	void *ptr = static_cast<void *>(value);
	if(set.count(ptr) == 0) {
		std::cout << "@context [\n";
		add_tabs(std::cout, level + 1);
		std::cout << "object: ";
		value->object->printvar(set, level + 1, ehi);
		add_tabs(std::cout, level + 1);
		std::cout << "scope: ";
		value->scope->printvar(set, level + 1, ehi);
		add_tabs(std::cout, level);
		std::cout << "]\n";
	} else {
		std::cout << "(recursion)\n";
	}
}

EH_METHOD(Node_Context, operator_colon) {
	ASSERT_NARGS(2, "Node.Context()");
	auto t = args->get<Tuple>();
	auto object = t->get(0);
	auto scope = t->get(1);
	scope->assert_type<Object>("Node.Context()", ehi);
	return Node_Context::make(ehcontext_t(object, scope), ehi->get_parent());
}

EH_METHOD(Node_Context, getObject) {
	ASSERT_RESOURCE(Node_Context, "Node.Context.getObject");
	return data->object;
}

EH_METHOD(Node_Context, getScope) {
	ASSERT_RESOURCE(Node_Context, "Node.Context.getScope");
	return data->scope;
}
