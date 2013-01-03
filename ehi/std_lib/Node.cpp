/*
 * Node
 * Represent a node of the AST.
 */
#include <sstream>

#include "std_lib_includes.hpp"
#include "Node.hpp"
#include "../eh.bison.hpp"

#define TOKEN(name, nparas) {name, {#name, nparas}}

const std::map<int, std::pair<const char *, int> > node_nparas {
	TOKEN(T_IF, 3),
	TOKEN(T_ELSE, 2),
	TOKEN(T_ELSIF, 2),
	TOKEN(T_WHILE, 2),
	TOKEN(T_FOR, 2),
	TOKEN(T_GIVEN, 2),
	TOKEN(T_MATCH, 2),
	TOKEN(T_END, 0),
	TOKEN(T_SWITCH, 2),
	TOKEN(T_DEFAULT, 1),
	TOKEN(T_CASE, 2),
	TOKEN(T_BREAK, 1),
	TOKEN(T_CONTINUE, 1),
	TOKEN(T_FUNC, 2),
	TOKEN(T_RET, 1),
	TOKEN(T_SEPARATOR, 2),
	TOKEN(T_NULL, 0),
	TOKEN(T_ENUM, 3),
	TOKEN(T_CLASS, 1),
	TOKEN(T_CLASS_MEMBER, 2),
	TOKEN(T_LITERAL, 1),
	TOKEN(T_TRY, 2),
	TOKEN(T_CATCH, 1),
	TOKEN(T_ATTRIBUTE, 2),
	TOKEN(T_ARRAY_MEMBER, 2),
	TOKEN(T_DOUBLEARROW, 2),
	TOKEN(T_CALL_METHOD, 3),
	TOKEN(T_TRY_FINALLY, 3),
	TOKEN(T_CATCH_IF, 2),
	TOKEN(T_FOR_IN, 3),
	TOKEN(T_NAMED_CLASS, 2),
	TOKEN(T_IF_ELSE, 4),
	TOKEN(T_NULLARY_ENUM, 1),
	TOKEN(T_ENUM_WITH_ARGUMENTS, 2),
	TOKEN(T_ARRAY_MEMBER_NO_KEY, 1),
	TOKEN(T_ANYTHING, 0),
	TOKEN(T_GROUPING, 1),
	TOKEN(T_ASSIGN, 2),
	TOKEN(T_ADD, 2),
	TOKEN(T_SUBTRACT, 2),
	TOKEN(T_MULTIPLY, 2),
	TOKEN(T_DIVIDE, 2),
	TOKEN(T_MODULO, 2),
	TOKEN(T_GREATER, 2),
	TOKEN(T_LESSER, 2),
	TOKEN(T_BINARY_AND, 2),
	TOKEN(T_BINARY_OR, 2),
	TOKEN(T_BINARY_XOR, 2),
	TOKEN(T_BINARY_COMPLEMENT, 1),
	TOKEN(T_NOT, 1),
	TOKEN(T_MATCH_SET, 1),
	TOKEN(T_COMMA, 2),
	TOKEN(T_ARRAY_LITERAL, 1),
	TOKEN(T_HASH_LITERAL, 1),
	TOKEN(T_CALL, 2),
	TOKEN(T_ACCESS, 2),
	TOKEN(T_COMMAND, 2),
	TOKEN(T_SHORTPARA, 2),
	TOKEN(T_LONGPARA, 2),
	TOKEN(T_VARIABLE, 1),
	TOKEN(T_CUSTOMOP, 3),
	TOKEN(T_XOR, 2),
	TOKEN(T_OR, 2),
	TOKEN(T_AND, 2),
	TOKEN(T_COMPARE, 2),
	TOKEN(T_EQ, 2),
	TOKEN(T_NE, 2),
	TOKEN(T_LE, 2),
	TOKEN(T_GE, 2),
	TOKEN(T_RIGHTSHIFT, 2),
	TOKEN(T_LEFTSHIFT, 2),
	TOKEN(T_ARROW, 2),
	TOKEN(T_RANGE, 2),
	TOKEN(T_SCOPE, 0),
	TOKEN(T_THIS, 0)
};
/*
 * The following tokens are not listed here, since they do not appear in the generated AST:
T_INTEGER, T_FLOAT, T_BOOL, T_AS, T_IN, T_FINALLY, T_STRING, T_RIGHTSHIFTEQ, T_LEFTSHIFTEQ, T_BINXOREQ, T_BINOREQ, T_BINANDEQ, T_XOREQ, T_OREQ, T_ANDEQ, T_MODULOEQ, T_DIVIDEEQ, T_MULTIPLYEQ, T_MINEQ, T_PLUSEQ, T_MINMIN, T_PLUSPLUS, T_NEGATIVE
 */

EH_INITIALIZER(Node) {
	REGISTER_METHOD(Node, execute);
	for(auto &kv : node_nparas) {
		obj->register_value(kv.second.first, Integer::make(kv.first), attributes_t::make_const());
	}
}

EH_METHOD(Node, execute) {
	ASSERT_RESOURCE(Node, "Node.execute");
	return ehi->eh_execute(obj->data(), ehi->global());
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

	Node::t *catch_op = paras[1]->get<Node>();
	for(; catch_op->op != T_END; catch_op = catch_op->paras[1]->get<Node>()) {
		Node::t *catch_block = catch_op->paras[0]->get<Node>();
		out << "\n";
		add_tabs(out, level);
		if(catch_block->op == T_CATCH) {
			out << "catch\n";
			add_tabs(out, level + 1);
			out << catch_block->paras[0]->decompile(level + 1);
		} else {
			// conditional catch (T_CATCH_IF)
			out << "catch if " << catch_block->paras[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << catch_block->paras[1]->decompile(level + 1);
		}
	}
}
static void decompile_match_like(std::ostringstream &out, const char *name, ehval_p *paras, int level) {
	out << name << " " << paras[0]->decompile(level);
	for(ehval_p node = paras[1]; node->get<Node>()->op != T_END; node = node->get<Node>()->paras[1]) {
		out << "\n";
		add_tabs(out, level + 1);
		Node::t *inner_op = node->get<Node>()->paras[0]->get<Node>();
		if(inner_op->op == T_DEFAULT) {
			out << "default\n";
			add_tabs(out, level + 2);
			out << inner_op->paras[0]->decompile(level + 2);
		} else {
			out << "case " << inner_op->paras[0]->decompile(level + 1) << "\n";
			add_tabs(out, level + 2);
			out << inner_op->paras[1]->decompile(level + 2);
		}
	}
	add_end(out, level);
}
static void decompile_if(std::ostringstream &out, ehval_p *paras, int level) {
	out << "if " << paras[0]->decompile(level) << "\n";
	add_tabs(out, level + 1);
	out << paras[1]->decompile(level + 1);
	for(Node::t *iop = paras[2]->get<Node>(); iop->op != T_END; iop = iop->paras[1]->get<Node>()) {
		ehval_p *current_block = iop->paras[0]->get<Node>()->paras;
		out << "\n";
		add_tabs(out, level);
		out << "elsif " << current_block[0]->decompile(level) << "\n";
		add_tabs(out, level + 1);
		out << current_block[1]->decompile(level);
	}
}

std::string Node::decompile(int level) {
	std::ostringstream out;
	Node::t *op = value;
	if(op == nullptr) {
		return "";
	}
	switch(op->op) {
		case T_LITERAL:
			out << op->paras[0]->decompile(level);
			break;
		case T_NULL:
			out << "()";
			break;
		case T_ANYTHING:
			out << "_";
			break;
		case T_ASSIGN:
			out << op->paras[0]->decompile(level) << " = " << op->paras[1]->decompile(level);
			break;
		case T_ADD:
			out << op->paras[0]->decompile(level) << " + " << op->paras[1]->decompile(level);
			break;
		case T_SUBTRACT:
			out << op->paras[0]->decompile(level) << " - " << op->paras[1]->decompile(level);
			break;
		case T_MULTIPLY:
			out << op->paras[0]->decompile(level) << " * " << op->paras[1]->decompile(level);
			break;
		case T_DIVIDE:
			out << op->paras[0]->decompile(level) << " / " << op->paras[1]->decompile(level);
			break;
		case T_MODULO:
			out << op->paras[0]->decompile(level) << " % " << op->paras[1]->decompile(level);
			break;
		case T_GREATER:
			out << op->paras[0]->decompile(level) << " > " << op->paras[1]->decompile(level);
			break;
		case T_LESSER:
			out << op->paras[0]->decompile(level) << " < " << op->paras[1]->decompile(level);
			break;
		case T_BINARY_OR:
			out << op->paras[0]->decompile(level) << " | " << op->paras[1]->decompile(level);
			break;
		case T_BINARY_XOR:
			out << op->paras[0]->decompile(level) << " ^ " << op->paras[1]->decompile(level);
			break;
		case T_BINARY_AND:
			out << op->paras[0]->decompile(level) << " & " << op->paras[1]->decompile(level);
			break;
		case T_COMMA:
			out << op->paras[0]->decompile(level) << ", " << op->paras[1]->decompile(level);
			break;
		case T_ACCESS:
			out << op->paras[0]->decompile(level) << "." << op->paras[1]->get<String>();
			break;
		case T_CALL_METHOD:
			out << op->paras[0]->decompile(level) << "." << op->paras[1]->get<String>() << "(" << op->paras[2]->decompile(level) << ")";
			break;
		case T_CALL:
			out << op->paras[0]->decompile(level) << " " << op->paras[1]->decompile(level);
			break;
		case T_NOT:
			out << "!" << op->paras[0]->decompile(level);
			break;
		case T_BINARY_COMPLEMENT:
			out << "~" << op->paras[0]->decompile(level);
			break;
		case T_VARIABLE:
			out << op->paras[0]->get<String>();
			break;
		case T_GROUPING:
			out << '(' << op->paras[0]->decompile(level) << ')';
			break;
		case T_RANGE:
			out << op->paras[0]->decompile(level) << " .. " << op->paras[1]->decompile(level);
			break;
		case T_GE:
			out << op->paras[0]->decompile(level) << " >= " << op->paras[1]->decompile(level);
			break;
		case T_LE:
			out << op->paras[0]->decompile(level) << " <= " << op->paras[1]->decompile(level);
			break;
		case T_NE:
			out << op->paras[0]->decompile(level) << " != " << op->paras[1]->decompile(level);
			break;
		case T_EQ:
			out << op->paras[0]->decompile(level) << " == " << op->paras[1]->decompile(level);
			break;
		case T_ARROW:
			// no space
			out << op->paras[0]->decompile(level) << "->" << op->paras[1]->decompile(level);
			break;
		case T_COMPARE:
			out << op->paras[0]->decompile(level) << " <=> " << op->paras[1]->decompile(level);
			break;
		case T_RIGHTSHIFT:
			out << op->paras[0]->decompile(level) << " >> " << op->paras[1]->decompile(level);
			break;
		case T_LEFTSHIFT:
			out << op->paras[0]->decompile(level) << " << " << op->paras[1]->decompile(level);
			break;
		case T_PLUSEQ:
			out << op->paras[0]->decompile(level) << " += " << op->paras[1]->decompile(level);
			break;
		case T_MINEQ:
			out << op->paras[0]->decompile(level) << " -= " << op->paras[1]->decompile(level);
			break;
		case T_MULTIPLYEQ:
			out << op->paras[0]->decompile(level) << " *= " << op->paras[1]->decompile(level);
			break;
		case T_DIVIDEEQ:
			out << op->paras[0]->decompile(level) << " /= " << op->paras[1]->decompile(level);
			break;
		case T_MODULOEQ:
			out << op->paras[0]->decompile(level) << " %= " << op->paras[1]->decompile(level);
			break;
		case T_ANDEQ:
			out << op->paras[0]->decompile(level) << " &&= " << op->paras[1]->decompile(level);
			break;
		case T_OREQ:
			out << op->paras[0]->decompile(level) << " ||= " << op->paras[1]->decompile(level);
			break;
		case T_XOREQ:
			out << op->paras[0]->decompile(level) << " ^^= " << op->paras[1]->decompile(level);
			break;
		case T_BINANDEQ:
			out << op->paras[0]->decompile(level) << " &= " << op->paras[1]->decompile(level);
			break;
		case T_BINOREQ:
			out << op->paras[0]->decompile(level) << " |= " << op->paras[1]->decompile(level);
			break;
		case T_BINXOREQ:
			out << op->paras[0]->decompile(level) << " ^= " << op->paras[1]->decompile(level);
			break;
		case T_RIGHTSHIFTEQ:
			out << op->paras[0]->decompile(level) << " >>= " << op->paras[1]->decompile(level);
			break;
		case T_LEFTSHIFTEQ:
			out << op->paras[0]->decompile(level) << " <<= " << op->paras[1]->decompile(level);
			break;
		case T_AND:
			out << op->paras[0]->decompile(level) << " && " << op->paras[1]->decompile(level);
			break;
		case T_OR:
			out << op->paras[0]->decompile(level) << " || " << op->paras[1]->decompile(level);
			break;
		case T_XOR:
			out << op->paras[0]->decompile(level) << " ^^ " << op->paras[1]->decompile(level);
			break;
		case T_CUSTOMOP:
			out << op->paras[0]->decompile(level) << " " <<
				op->paras[1]->get<String>() << " " <<
				op->paras[2]->decompile(level);
			break;
		case T_SEPARATOR: {
			out << op->paras[0]->decompile(level);
			ehval_p r = op->paras[1];
			if(!r->is_a<Node>() || r->get<Node>()->op != T_SEPARATOR || r->get<Node>()->op != T_END) {
				out << "\n";
				add_tabs(out, level);
				out << op->paras[1]->decompile(level);
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
			out << "ret " << op->paras[0]->decompile(level);
			break;
		case T_CLASS_MEMBER:
			out << op->paras[0]->decompile(level) << op->paras[1]->decompile(level);
			break;
		case T_ATTRIBUTE:
			out << op->paras[0]->decompile(level) << " " << op->paras[1]->decompile(level);
			break;
		case T_IF:
			decompile_if(out, op->paras, level);
			add_end(out, level);
			break;
		case T_IF_ELSE:
			decompile_if(out, op->paras, level);
			out << "\n";
			add_tabs(out, level);
			out << "else\n";
			add_tabs(out, level + 1);
			out << op->paras[3]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_WHILE:
			out << "while " << op->paras[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->paras[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_TRY:
			decompile_try_catch(out, op->paras, level);
			add_end(out, level);
			break;
		case T_TRY_FINALLY:
			decompile_try_catch(out, op->paras, level);
			out << "\n";
			add_tabs(out, level);
			out << "finally\n";
			add_tabs(out, level + 1);
			out << op->paras[2]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_NAMED_CLASS:
			out << "class " << op->paras[0]->get<String>() << "\n";
			add_tabs(out, level + 1);
			out << op->paras[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_ENUM:
			out << "enum " << op->paras[0]->get<String>() << "\n";
			add_tabs(out, level + 1);
			// decompile member list
			for(ehval_p node = op->paras[1]; ; node = node->get<Node>()->paras[0]) {
				ehval_p current_member;
				bool is_last;
				if(node->get<Node>()->op == T_COMMA) {
					current_member = node->get<Node>()->paras[1];
					is_last = false;
				} else {
					current_member = node;
					is_last = true;
				}

				// handle the member
				out << current_member->get<Node>()->paras[0]->get<String>();
				if(current_member->get<Node>()->op == T_ENUM_WITH_ARGUMENTS) {
					out << "(";
					for(ehval_p argument = current_member->get<Node>()->paras[1]; ; argument = argument->get<Node>()->paras[1]) {
						ehval_p name = argument->is_a<Node>() ? argument->get<Node>()->paras[0] : argument;
						out << name->get<String>();
						if(!argument->is_a<Node>() || argument->get<Node>()->op != T_COMMA) {
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
			out << op->paras[2]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_CLASS:
			out << "class\n";
			add_tabs(out, level + 1);
			out << op->paras[0]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_FUNC:
			out << "func: " << op->paras[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->paras[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_SWITCH:
			decompile_match_like(out, "switch", op->paras, level);
			break;
		case T_MATCH:
			decompile_match_like(out, "match", op->paras, level);
			break;
		case T_GIVEN:
			decompile_match_like(out, "given", op->paras, level);
			break;
		case T_FOR:
			out << "for " << op->paras[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->paras[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_FOR_IN:
			out << "for " << op->paras[0]->decompile(level) << " in " << op->paras[1]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->paras[2]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_ARRAY_LITERAL:
			out << "[";
			for(ehval_p n = op->paras[0]; n->get<Node>()->op != T_END; n = n->get<Node>()->paras[0]) {
				Node::t *member_op = n->get<Node>()->paras[1]->get<Node>();
				out << member_op->paras[0]->decompile(level);
				if(member_op->op == T_ARRAY_MEMBER) {
					out << " => " << member_op->paras[1]->decompile(level);
				}
				if(n->get<Node>()->paras[0]->get<Node>()->op != T_END) {
					out << ", ";
				}
			}
			out << "]";
			break;
		case T_MATCH_SET:
			out << "@";
			out << op->paras[0]->get<String>();
			break;
		case T_HASH_LITERAL:
			out << "{";
			for(ehval_p n = op->paras[0]; n->get<Node>()->op != T_END; n = n->get<Node>()->paras[0]) {
				Node::t *member_op = n->get<Node>()->paras[1]->get<Node>();
				out << member_op->paras[0]->decompile(level);
				out << ": " << member_op->paras[1]->decompile(level);
				if(n->get<Node>()->paras[0]->get<Node>()->op != T_END) {
					out << ", ";
				}
			}
			out << "}";
			break;
		case T_COMMAND:
			out << "$" << op->paras[0]->get<String>();
			for(ehval_p node = op->paras[1]; node->get<Node>()->op != T_END; node = node->get<Node>()->paras[1]) {
				Node::t *node2 = node->get<Node>()->paras[0]->get<Node>();
				switch(node2->op) {
					case T_SHORTPARA:
						out << " -" << node2->paras[0]->decompile(level);
						out << "=" << node2->paras[1]->decompile(level);
						break;
					case T_LONGPARA:
						out << " --" << node2->paras[0]->decompile(level);
						out << "=" << node2->paras[1]->decompile(level);
						break;
					default:
						out << " " << node->get<Node>()->paras[0]->decompile(level);
						break;
				}
			}
			out << "\n";
			break;
		default:
			out << "(cannot decode value: " << op->op << ")";
			break;
	}
	return out.str();
}

Node::t *eh_addnode(int opcode) {
	return new Node::t(opcode, 0);
}
Node::t *eh_addnode(int opcode, ehval_p first) {
	Node::t *op = new Node::t(opcode, 1);
	op->paras[0] = first;
	return op;
}
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second) {
	Node::t *op = new Node::t(opcode, 2);
	op->paras[0] = first;
	op->paras[1] = second;
	return op;
}
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third) {
	Node::t *op = new Node::t(opcode, 3);
	op->paras[0] = first;
	op->paras[1] = second;
	op->paras[2] = third;
	return op;
}
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third, ehval_p fourth) {
	Node::t *op = new Node::t(opcode, 4);
	op->paras[0] = first;
	op->paras[1] = second;
	op->paras[2] = third;
	op->paras[3] = fourth;
	return op;
}
