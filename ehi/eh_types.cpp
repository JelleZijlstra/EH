#include <set>
#include <sstream>

#include "eh.h"
#include "eh_libclasses.h"
#include "std_lib/Array.h"
#include "std_lib/Function.h"
#include "std_lib/Hash.h"
#include "std_lib/Range.h"
#include "std_lib/Tuple.h"
#include "std_lib/SuperClass.h"

/*
 * ehretval_t
 */
void ehretval_t::print() {
	std::cout << get_typestring(this->type()) << std::endl;
	switch(this->type()) {
		case string_e:
			printf("%s", this->get_stringval());
			break;
		case int_e:
			printf("%d", this->get_intval());
			break;
		case bool_e:
			if(this->get_boolval()) {
				printf("(true)");
			} else {
				printf("(false)");
			}
			break;
		case null_e:
			printf("(null)");
			break;
		case float_e:
			printf("%f", this->get_floatval());
			break;
		case range_e:
			this->get_rangeval()->min->print();
			printf(" to ");
			this->get_rangeval()->max->print();
			break;
		case object_e:
			std::cout << get_typestring((type_enum) (this->get_objectval()->type_id)) << std::endl;
		default:
			printf("(cannot print value)");
			break;
	}
	std::cout << std::endl;
	return;
}
std::list<ehretval_p> ehretval_t::children() {
	std::list<ehretval_p> out;
	switch(this->type()) {
		case object_e: {
			ehobj_t *o = this->get_objectval();
			OBJECT_FOR_EACH(o, i) {
				out.push_back(i->second->value);
			}
			out.push_back(o->parent);
			out.push_back(o->object_data);
			out.push_back(o->real_parent);
			for(std::list<ehretval_p>::iterator i = o->super.begin(), end = o->super.end(); i != end; i++) {
				out.push_back(*i);
			}
			break;
		}
		case array_e:
			ARRAY_FOR_EACH_INT(this->get_arrayval(), i) {
				out.push_back(i->second);
			}
			ARRAY_FOR_EACH_STRING(this->get_arrayval(), i) {
				out.push_back(i->second);
			}
			break;
		case range_e:
			out.push_back(this->get_rangeval()->min);
			out.push_back(this->get_rangeval()->max);
			break;
		case binding_e:
			out.push_back(this->get_bindingval()->method);
			out.push_back(this->get_bindingval()->object_data);
			break;
		case hash_e: {
			ehhash_t *f = this->get_hashval();
			HASH_FOR_EACH(f, i) {
				out.push_back(i->second);
			}
			break;
		}
		case tuple_e: {
			ehtuple_t *t = this->get_tupleval();
			int size = t->size();
			for(int i = 0; i < size; i++) {
				out.push_back(t->get(i));
			}
			break;
		}
		case super_class_e:
			out.push_back(this->get_super_classval()->content());
			break;
		case resource_e:
			break;
		default:
			// nothing to see here
			break;
	}
	return out;
}
bool ehretval_t::equals(ehretval_p rhs) {
	if(this->type() != rhs->type()) {
		return false;
	}
	switch(this->type()) {
		case int_e:
			return (this->get_intval() == rhs->get_intval());
		case string_e:
			return strcmp(this->get_stringval(), rhs->get_stringval()) == 0;
		case bool_e:
			return (this->get_boolval() == rhs->get_boolval());
		case null_e:
			// null always equals null
			return true;
		case float_e:
			return (this->get_floatval() == rhs->get_floatval());
		case range_e:
			return this->get_rangeval()->min->equals(rhs->get_rangeval()->min)
				&& this->get_rangeval()->max->equals(rhs->get_rangeval()->max);
		case resource_e:
			return this->get_resourceval() == rhs->get_resourceval();
		default:
			// TODO: array comparison
			return false;
	}
}
bool ehretval_t::is_a(int in) {
	if(this->type() == in) {
		return true;
	} else {
		return this->type() == object_e && this->get_objectval()->type_id == in;
	}
}
bool ehretval_t::inherited_is_a(int in) {
	if(this->type() == in) {
		return true;
	} else if(this->type() == object_e) {
		ehobj_t *obj = this->get_objectval();
		if(obj->type_id == in) {
			return true;
		} else {
			for(std::list<ehretval_p>::const_iterator i = obj->super.begin(), end = obj->super.end(); i != end; i++) {
				if((*i)->inherited_is_a(in)) {
					return true;
				}
			}
		}
	}
	return false;
}
int ehretval_t::get_full_type() const  {
	int out = this->type();
	if(out == object_e) {
		return this->get_objectval()->type_id;
	} else {
		return out;
	}
}
const std::string &ehretval_t::type_string(EHI *ehi) const {
	int type = this->get_full_type();
	return ehi->get_parent()->repo.get_name(type);
}
ehretval_p ehretval_t::self_or_data(const ehretval_p in) {
	if(in->type() == object_e) {
		return in->get_objectval()->object_data;
	} else {
		return in;
	}
}
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
					out << " = " << op->paras[2]->decompile(level);
					break;
				case T_ATTRIBUTE:
					if(op->nparas != 0) {
						out << op->paras[0]->decompile(level) << op->paras[1]->decompile(level) << " ";
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
ehretval_t::~ehretval_t() {
	switch(_type) {
		// Simple types; nothing to do
		case int_e:
		case bool_e:
		case float_e:
		case type_e:
		case null_e:
		case attribute_e:
		case attributestr_e:
			break;
		case super_class_e:
			delete this->super_classval;
			break;
		case op_e:
			delete this->opval;
			break;
		case string_e:
			delete[] this->stringval;
			break;
		// Delete object. An ehretval_t owns the object pointed to.
		case range_e:
			delete this->rangeval;
			break;
		case object_e:
			delete this->objectval;
			break;
		case func_e:
			delete this->funcval;
			break;
		case array_e:
			delete this->arrayval;
			break;
		case resource_e:
			delete this->resourceval;
			break;
		case binding_e:
			delete this->bindingval;
			break;
		case hash_e:
			delete this->hashval;
			break;
		case tuple_e:
		 	delete this->tupleval;
		 	break;
	}
}

/*
 * ehobj_t
 */
ehmember_p ehobj_t::get_recursive(const char *name, const ehcontext_t context) {
	if(this->has(name)) {
		return this->members[name];
	} else if(this->real_parent == NULL) {
		if(this->parent != NULL) {
			return this->get_parent()->get_recursive(name, context);
		} else {
			return NULL;
		}
	} else {
		if(this->parent != NULL && this->get_parent()->has(name)) {
			return this->get_parent()->members[name];
		} else {
			return this->get_real_parent()->get_recursive(name, context);
		}
	}
}
bool ehobj_t::inherited_has(const std::string &key) const {
	if(this->has(key)) {
		return true;
	}
	for(std::list<ehretval_p>::const_iterator i = super.begin(), end = super.end(); i != end; i++) {
		if((*i)->get_objectval()->inherited_has(key)) {
			return true;
		}
	}
	return false;
}
std::set<std::string> ehobj_t::member_set() {
	std::set<std::string> out;
	OBJECT_FOR_EACH(this, i) {
		out.insert(i->first);
	}
	for(std::list<ehretval_p>::const_iterator i = super.begin(), end = super.end(); i != end; i++) {
		std::set<std::string> member_set = (*i)->get_objectval()->member_set();
		for(std::set<std::string>::iterator j = member_set.begin(), iend = member_set.end(); j != iend; j++) {
			out.insert(*j);
		}
	}
	return out;
}
ehmember_p ehobj_t::inherited_get(const std::string &key) {
	if(this->has(key)) {
		return this->get_known(key);
	}
	for(std::list<ehretval_p>::const_iterator i = super.begin(), end = super.end(); i != end; i++) {
		ehmember_p result = (*i)->get_objectval()->inherited_get(key);
		if(!ehmember_p::null(result)) {
			return result;
		}
	}
	return NULL;
}
bool ehobj_t::context_compare(const ehcontext_t key) const {
	// in global context, we never have access to private stuff
	if(ehretval_p::null(key.object)) {
		return false;
	} else if(key.scope->get_objectval() == this) {
		return true;
	} else if(this->type_id == key.object->get_full_type()) {
		return true;
	} else if(key.object->type() == object_e) {
		return this->context_compare(key.object->get_objectval()->parent);
	} else {
		return false;
	}
}
void ehobj_t::register_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, class EHInterpreter *interpreter_parent) {
	ehretval_p func = interpreter_parent->make_method(method);
	this->register_value(name, func, attributes);
}
void ehobj_t::register_value(const std::string &name, ehretval_p value, const attributes_t attributes) {
	ehmember_p member;
	member->attribute = attributes;
	member->value = value;
	this->insert(name, member);
}
void ehobj_t::register_member_class(const std::string &name, const int new_type_id, const ehobj_t::initializer init_func, const attributes_t attributes, class EHInterpreter *interpreter_parent, ehretval_p the_class) {
	ehobj_t *newclass;
	ehretval_p new_value;
	if(the_class == NULL) {
		newclass = new ehobj_t();
		new_value = interpreter_parent->make_object(newclass);
	} else {
		newclass = the_class->get_objectval();
		new_value = the_class;
	}
	// register class
	if(new_type_id == -1) {
		newclass->type_id = interpreter_parent->repo.register_class(name, new_value);
	} else {
		newclass->type_id = new_type_id;
		interpreter_parent->repo.register_known_class(new_type_id, name, new_value);
	}
	if(name != "GlobalObject") {
		newclass->parent = interpreter_parent->global_object;
	}

	// inherit from Object, except in Object itself
	if(new_type_id != object_e) {
		newclass->inherit(interpreter_parent->base_object);
	}
	init_func(newclass, interpreter_parent);
	ehmember_p member;
	member->attribute = attributes;
	member->value = new_value;
	this->insert(name, member);
}

ehobj_t::~ehobj_t() {
	// Commenting out for now until I figure out how to get it working.
	//ehi->call_method_obj(this, "finalize", 0, NULL, NULL);
}

// eh_exception

eh_exception::~eh_exception() throw() {}
