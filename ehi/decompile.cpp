/*
 * decompile.cpp
 * Decompile a piece of EH AST into EH code.
 */
#include <sstream>

#include "eh.hpp"

static void add_end(std::ostringstream &out, int levels) {
	out << "\n";
	add_tabs(out, levels);
	out << "end";
}
std::string Node::decompile(int level) {
	std::ostringstream out;
	Node::t *op = value;
	switch(op->op) {
		case T_LITERAL:
			out << op->paras[0]->decompile(level);
			break;
		case T_NULL:
			out << "()";
			break;
		case '_':
			out << "_";
			break;
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
		case '=':
		case '<':
		case '>':
		case '|':
		case '^':
		case '&':
			// binary ops
			out << op->paras[0]->decompile(level) << " " << char(op->op) << " " << op->paras[1]->decompile(level);
			break;
		case ',':
			out << op->paras[0]->decompile(level) << ", " << op->paras[1]->decompile(level);
			break;
		case '.':
			out << op->paras[0]->decompile(level) << "." << op->paras[1]->get<String>();
			break;
		case ':':
			out << op->paras[0]->decompile(level) << " " << op->paras[1]->decompile(level);
			break;
		case '!':
		case '~':
			// unary ops
			out << char(op->op) << op->paras[0]->decompile(level);
			break;
		case '$':
			out << op->paras[0]->get<String>();
			break;
		case '(':
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
		case T_SEPARATOR:
			if(op->nparas != 0) {
				out << op->paras[0]->decompile(level);
				ehval_p r = op->paras[1];
				if(!r->is_a<Node>() || r->get<Node>()->op != T_SEPARATOR || r->get<Node>()->nparas != 0) {
					out << "\n";
					add_tabs(out, level);
					out << op->paras[1]->decompile(level);
				}
			}
			break;
		case T_THIS:
			out << "this";
			break;
		case T_SCOPE:
			out << "scope";
			break;
		case T_RET:
			if(op->nparas == 0) {
				out << "ret";
			} else {
				out << "ret " << op->paras[0]->decompile(level);
			}
			break;
		case T_CLASSMEMBER:
			out << op->paras[0]->decompile(level) << op->paras[1]->decompile(level);
			break;
		case T_ATTRIBUTE:
			if(op->nparas != 0) {
				out << op->paras[0]->decompile(level) << " " << op->paras[1]->decompile(level);
			}
			break;
		case T_IF:
			out << "if " << op->paras[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->paras[1]->decompile(level + 1);
			if(op->nparas > 2) {
				for(Node::t *iop = op->paras[2]->get<Node>(); iop->nparas != 0; iop = iop->paras[1]->get<Node>()) {
					ehval_p *current_block = iop->paras[0]->get<Node>()->paras;
					out << "\n";
					add_tabs(out, level);
					out << "elsif " << current_block[0]->decompile(level) << "\n";
					add_tabs(out, level + 1);
					out << current_block[1]->decompile(level);
				}
				if(op->nparas == 4) {
					out << "\n";
					add_tabs(out, level);
					out << "else\n";
					add_tabs(out, level + 1);
					out << op->paras[3]->decompile(level + 1);
				}
			}
			add_end(out, level);
			break;
		case T_WHILE:
			out << "while " << op->paras[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->paras[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_TRY: {
			out << "try\n";
			add_tabs(out, level + 1);
			out << op->paras[0]->decompile(level + 1);

			Node::t *catch_op = op->paras[1]->get<Node>();
			for(; catch_op->nparas != 0; catch_op = catch_op->paras[1]->get<Node>()) {
				Node::t *catch_block = catch_op->paras[0]->get<Node>();
				out << "\n";
				add_tabs(out, level);
				if(catch_block->nparas == 1) {
					out << "catch\n";
					add_tabs(out, level + 1);
					out << catch_block->paras[0]->decompile(level + 1);
				} else {
					// conditional catch
					out << "catch if " << catch_block->paras[0]->decompile(level) << "\n";
					add_tabs(out, level + 1);
					out << catch_block->paras[1]->decompile(level + 1);
				}
			}

			if(op->nparas == 3) {
				out << "\n";
				add_tabs(out, level);
				out << "finally\n";
				add_tabs(out, level + 1);
				out << op->paras[2]->decompile(level + 1);
			}
			add_end(out, level);
			break;
		}
		case T_CLASS:
			if(op->nparas == 2) {
				out << "class " << op->paras[0]->get<String>() << "\n";
				add_tabs(out, level + 1);
				out << op->paras[1]->decompile(level + 1);
			} else {
				out << "class\n";
				add_tabs(out, level + 1);
				out << op->paras[0]->decompile(level + 1);
			}
			add_end(out, level);
			break;
		case T_FUNC:
			out << "func: " << op->paras[0]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->paras[1]->decompile(level + 1);
			add_end(out, level);
			break;
		case T_MATCH:
			out << "match " << op->paras[0]->decompile(level);
			for(ehval_p node = op->paras[1]; node->get<Node>()->nparas != 0; node = node->get<Node>()->paras[1]) {
				out << "\n";
				add_tabs(out, level + 1);
				Node::t *inner_op = node->get<Node>()->paras[0]->get<Node>();
				if(inner_op->nparas == 1) {
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
			break;
		case T_GIVEN:
			out << "given " << op->paras[0]->decompile(level);
			for(ehval_p node = op->paras[1]; node->get<Node>()->nparas != 0; node = node->get<Node>()->paras[1]) {
				out << "\n";
				add_tabs(out, level + 1);
				Node::t *inner_op = node->get<Node>()->paras[0]->get<Node>();
				if(inner_op->nparas == 1) {
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
			break;
		case T_IN:
			out << "for " << op->paras[0]->decompile(level) << " in " << op->paras[1]->decompile(level) << "\n";
			add_tabs(out, level + 1);
			out << op->paras[2]->decompile(level + 1);
			add_end(out, level);
			break;
		case '[':
			out << "[";
			for(ehval_p n = op->paras[0]; n->get<Node>()->nparas != 0; n = n->get<Node>()->paras[0]) {
				Node::t *member_op = n->get<Node>()->paras[1]->get<Node>();
				out << member_op->paras[0]->decompile(level);
				if(member_op->nparas != 1) {
					out << " => " << member_op->paras[1]->decompile(level);
				}
				if(n->get<Node>()->paras[0]->get<Node>()->nparas != 0) {
					out << ", ";
				}
			}
			out << "]";
			break;
		case '@':
			out << "@";
			if(op->nparas == 1) {
				out << op->paras[0]->get<String>();
			} else {
				out << op->paras[0]->decompile(level) << " ";
				out << op->paras[1]->decompile(level);
			}
			break;
		case '{':
			out << "{";
			for(ehval_p n = op->paras[0]; n->get<Node>()->nparas != 0; n = n->get<Node>()->paras[0]) {
				Node::t *member_op = n->get<Node>()->paras[1]->get<Node>();
				out << member_op->paras[0]->decompile(level);
				if(member_op->nparas != 1) {
					out << ": " << member_op->paras[1]->decompile(level);
				}
				if(n->get<Node>()->paras[0]->get<Node>()->nparas != 0) {
					out << ", ";
				}
			}
			out << "}";
			break;
		case T_COMMAND:
			out << "$" << op->paras[0]->get<String>();
			for(ehval_p node = op->paras[1]; node->get<Node>()->nparas != 0; node = node->get<Node>()->paras[1]) {
				Node::t *node2 = node->get<Node>()->paras[0]->get<Node>();
				switch(node2->op) {
					case T_SHORTPARA:
						out << " -" << node2->paras[0]->decompile(level);
						if(node2->nparas == 2) {
							out << "=" << node2->paras[1]->decompile(level);
						}
						break;
					case T_LONGPARA:
						out << " --" << node2->paras[0]->decompile(level);
						if(node2->nparas == 2) {
							out << "=" << node2->paras[1]->decompile(level);
						}
						break;
					case T_REDIRECT:
						out << " > " << node2->paras[0]->decompile(level);
						break;
					case '}':
						out << " } " << node2->paras[0]->decompile(level);
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
