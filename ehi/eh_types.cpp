#include <set>
#include <stdio.h>
#include <string.h>

#include "eh.hpp"
#include "eh_libclasses.hpp"
#include "std_lib/Array.hpp"
#include "std_lib/Function.hpp"
#include "std_lib/Hash.hpp"
#include "std_lib/Range.hpp"
#include "std_lib/Tuple.hpp"
#include "std_lib/SuperClass.hpp"

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
bool ehretval_t::is_a(unsigned int in) {
	unsigned int type = this->extended_type();
	if(type == in || ((in == resource_e && type >= type_repository::first_user_type))) {
		return true;
	} else {
		return type == object_e && this->get_objectval()->type_id == in;
	}
}
bool ehretval_t::inherited_is_a(unsigned int in) {
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
unsigned int ehretval_t::get_full_type() const  {
	unsigned int out = this->extended_type();
	if(out == object_e) {
		return this->get_objectval()->type_id;
	} else {
		return out;
	}
}
const std::string &ehretval_t::type_string(EHI *ehi) const {
	unsigned int type = this->get_full_type();
	return ehi->get_parent()->repo.get_name(type);
}
ehretval_p ehretval_t::self_or_data(const ehretval_p in) {
	if(in->type() == object_e) {
		return in->get_objectval()->object_data;
	} else {
		return in;
	}
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
	} else if(this->parent != NULL) {
		return this->get_parent()->get_recursive(name, context);
	} else {
		return NULL;
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
bool ehobj_t::context_compare(const ehcontext_t &key) const {
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
int ehobj_t::register_member_class(const std::string &name, const int new_type_id, const ehobj_t::initializer init_func, const attributes_t attributes, class EHInterpreter *interpreter_parent, ehretval_p the_class) {
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
	// inherit from Object, except in Object itself
	if(new_type_id != object_e && name != "GlobalObject") {
		newclass->inherit(interpreter_parent->base_object);
	}
	if(name != "GlobalObject") {
		newclass->parent = interpreter_parent->global_object;
	}
	init_func(newclass, interpreter_parent);
	// inherit from Object, except in Object itself
	ehmember_p member;
	member->attribute = attributes;
	member->value = new_value;
	this->insert(name, member);
	return newclass->type_id;
}

ehobj_t::~ehobj_t() {
	// Commenting out for now until I figure out how to get it working.
	//ehi->call_method_obj(this, "finalize", 0, NULL, NULL);
}

// eh_exception

eh_exception::~eh_exception() throw() {}
