/*
 * decompile.cpp
 * Decompile a piece of EH AST into EH code.
 */
#include <sstream>

#include "eh.hpp"

static void add_tabs(std::ostringstream &out, int levels) {
	for(int i = 0; i < levels; i++) {
		out << "\t";
	}
}
static void add_end(std::ostringstream &out, int levels) {
	out << "\n";
	add_tabs(out, levels);
	out << "end";
}
std::string ehretval_t::decompile(int level) {
	std::ostringstream out;
	switch(this->type()) {
		case int_e:
			out << this->get_intval();
			break;
		case float_e:
			out << this->get_floatval();
			break;
		case bool_e:
			if(this->get_boolval()) {
				out << "true";
			} else {
				out << "false";
			}
			break;
		case null_e:
			out << "()";
			break;
		case string_e:
			out << '"' << this->get_stringval() << '"';
			break;
		case func_e: {
			ehfunc_t *f = this->get_funcval();
			if(f->type == lib_e) {
				out << "(args) => (native code)";
			} else {
				out << "func: " << f->args->decompile(level) << "\n";
				add_tabs(out, level + 1);
				out << f->code->decompile(level + 1);
				add_end(out, level);
			}
			break;
		}
		case attribute_e:
			switch(this->get_attributeval()) {
				case publica_e:
					out << "public";
					break;
				case privatea_e:
					out << "private";
					break;
				case statica_e:
					out << "static";
					break;
				case consta_e:
					out << "const";
					break;
			}
			break;
		case op_e: {
			opnode_t *op = this->get_opval();
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
					out << op->paras[0]->decompile(level) << "." << op->paras[1]->get_stringval();
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
					out << op->paras[0]->get_stringval();
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
						op->paras[1]->get_stringval() << " " <<
						op->paras[2]->decompile(level);
					break;
				case T_SEPARATOR:
					if(op->nparas != 0) {
						out << op->paras[0]->decompile(level);
						ehretval_p r = op->paras[1];
						if(r->type() != op_e || r->get_opval()->op != T_SEPARATOR || r->get_opval()->nparas != 0) {
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
						for(opnode_t *iop = op->paras[2]->get_opval(); iop->nparas != 0; iop = iop->paras[1]->get_opval()) {
							ehretval_p *current_block = iop->paras[0]->get_opval()->paras;
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

					opnode_t *catch_op = op->paras[1]->get_opval();
					for(; catch_op->nparas != 0; catch_op = catch_op->paras[1]->get_opval()) {
						opnode_t *catch_block = catch_op->paras[0]->get_opval();
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
						out << "class " << op->paras[0]->get_stringval() << "\n";
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
					for(ehretval_p node = op->paras[1]; node->get_opval()->nparas != 0; node = node->get_opval()->paras[1]) {
						out << "\n";
						add_tabs(out, level + 1);
						opnode_t *inner_op = node->get_opval()->paras[0]->get_opval();
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
					for(ehretval_p node = op->paras[1]; node->get_opval()->nparas != 0; node = node->get_opval()->paras[1]) {
						out << "\n";
						add_tabs(out, level + 1);
						opnode_t *inner_op = node->get_opval()->paras[0]->get_opval();
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
					for(ehretval_p n = op->paras[0]; n->get_opval()->nparas != 0; n = n->get_opval()->paras[0]) {
						opnode_t *member_op = n->get_opval()->paras[1]->get_opval();
						out << member_op->paras[0]->decompile(level);
						if(member_op->nparas != 1) {
							out << " => " << member_op->paras[1]->decompile(level);
						}
						if(n->get_opval()->paras[0]->get_opval()->nparas != 0) {
							out << ", ";
						}
					}
					out << "]";
					break;
				case '@':
					out << "@";
					if(op->nparas == 1) {
						out << op->paras[0]->get_stringval();
					} else {
						out << op->paras[0]->decompile(level) << " ";
						out << op->paras[1]->decompile(level);
					}
					break;
				case '{':
					out << "{";
					for(ehretval_p n = op->paras[0]; n->get_opval()->nparas != 0; n = n->get_opval()->paras[0]) {
						opnode_t *member_op = n->get_opval()->paras[1]->get_opval();
						out << member_op->paras[0]->decompile(level);
						if(member_op->nparas != 1) {
							out << ": " << member_op->paras[1]->decompile(level);
						}
						if(n->get_opval()->paras[0]->get_opval()->nparas != 0) {
							out << ", ";
						}
					}
					out << "}";
					break;
				case T_COMMAND:
					out << "$" << op->paras[0]->get_stringval();
					for(ehretval_p node = op->paras[1]; node->get_opval()->nparas != 0; node = node->get_opval()->paras[1]) {
						opnode_t *node2 = node->get_opval()->paras[0]->get_opval();
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
								out << " " << node->get_opval()->paras[0]->decompile(level);
								break;
						}
					}
					out << "\n";
					break;
				default:
					out << "(cannot decode value: " << op->op << ")";
					break;
			}
			break;
		}
		default:
			// should not appear in AST, and thus cannot be decompiled
			assert(false);
	}
	return out.str();
}
