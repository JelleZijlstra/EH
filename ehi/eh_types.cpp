#include <set>
#include <stdio.h>
#include <string.h>

#include "eh.hpp"
#include "eh_libclasses.hpp"
#include "std_lib/Array.hpp"
#include "std_lib/Function.hpp"
#include "std_lib/Binding.hpp"
#include "std_lib/Hash.hpp"
#include "std_lib/Range.hpp"
#include "std_lib/Tuple.hpp"
#include "std_lib/SuperClass.hpp"
#include "std_lib/ConstError.hpp"
#include "std_lib/VisibilityError.hpp"
#include "std_lib/NameError.hpp"

ehmember_p ehval_t::set_property(const char *name, ehval_p value, ehcontext_t context, EHI *ehi) {
	// caller should ensure object is actually an object
	ehobj_t *obj = get<Object>();
	// unbind bindings to myself
	if(value->is_a<Binding>()) {
		ehval_p obj_data = value->get<Binding>()->object_data;
		if(obj_data->is_a<Object>() && obj == obj_data->get<Object>()) {
			ehval_p reference_retainer = value;
			value = value->get<Binding>()->method;
		}
	}
	ehmember_p result = obj->inherited_get(name);
	if(ehmember_p::null(result)) {
		ehmember_p new_member;
		new_member->value = value;
		obj->insert(name, new_member);
		return new_member;
	} else if(result->attribute.isconst == const_e) {
		throw_ConstError(this, name, ehi);
	} else if(result->attribute.visibility == private_e && !obj->context_compare(context, ehi)) {
		throw_VisibilityError(this, name, ehi);
	} else if(result->attribute.isstatic == static_e) {
		result->value = value;
		return result;
	} else {
		// set in this object
		ehmember_p new_member = ehmember_t::make(result->attribute, value);
		obj->insert(name, new_member);
		return new_member;
	}
	return result;
}
// insert an ehmember_p directly on this object, without worrying about inheritance
ehmember_p ehval_t::set_member(const char *name, ehmember_p member, ehcontext_t context, EHI *ehi) {
	// caller should ensure object is actually an object
	ehobj_t *obj = get<Object>();
	// unbind bindings to myself
	ehval_p value = member->value;
	if(value->is_a<Binding>()) {
		ehval_p obj_data = value->get<Binding>()->object_data;
		if(obj_data->is_a<Object>() && obj == obj_data->get<Object>()) {
			member->value = value->get<Binding>()->method;
		}
	}
	if(obj->has(name)) {
		ehmember_p the_member = obj->get_known(name);
		if(the_member->attribute.isconst == const_e) {
			throw_ConstError(this, name, ehi);
		}
		if(the_member->attribute.visibility == private_e && !obj->context_compare(context, ehi)) {
			// pretend private members don't exist
			throw_VisibilityError(this, name, ehi);
		}
	}
	obj->insert(name, member);
	return member;
}
// get a property of the given name, without creating a binding
ehval_p ehval_t::get_property_no_binding(const char *name, ehcontext_t context, EHI *ehi) {
	ehval_p object;
	if(is_a<Object>()) {
		object = this;
	} else {
		object = ehi->get_parent()->repo.get_object(this);
	}
	ehobj_t *obj = object->get<Object>();
	ehmember_p member = obj->inherited_get(name);
	if(member.null()) {
		throw_NameError(this, name, ehi);
	} else if (member->attribute.visibility == private_e && !obj->context_compare(context, ehi)) {
		throw_VisibilityError(this, name, ehi);
	}
	return member->value;
}
// get a property of the given name from the base_var object, creating a binding if necessary
ehval_p ehval_t::get_property(const char *name, ehcontext_t context, EHI *ehi) {
	ehval_p out = get_property_no_binding(name, context, ehi);
	if(out->deep_is_a<Function>()) {
		return Binding::make(this, out, ehi->get_parent());
	} else {
		return out;
	}
}

/*
 * ehobj_t
 */
ehval_p Object::make(ehobj_t *obj, class EHInterpreter *parent) {
	return parent->allocate<Object>(obj);
}

void Object::printvar(printvar_set &set, int level, EHI *ehi) {
	if(this->deep_is_a<Function>()) {
		value->object_data->printvar(set, level, ehi);
	} else {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			const std::string name = ehi->get_parent()->repo.get_name(this);
			std::cout << "@object <" << name << "> [" << std::endl;
			for(auto &i : value->members) {
				add_tabs(std::cout, level + 1);
				std::cout << i.first << " <";
				const attributes_t attribs = i.second->attribute;
				std::cout << (attribs.visibility == public_e ? "public" : "private") << ",";
				std::cout << (attribs.isstatic == static_e ? "static" : "non-static") << ",";
				std::cout << (attribs.isconst == const_e ? "constant" : "non-constant") << ">: ";
				PRINTVAR(i.second->value, level + 1);
			}
			add_tabs(std::cout, level);
			std::cout << "]" << std::endl;
		} else {
			std::cout << "(recursion)" << std::endl;
		}
	}
}
ehmember_p ehobj_t::get_recursive(const char *name, const ehcontext_t context) {
	if(this->has(name)) {
		return this->members[name];
	} else if(this->parent != nullptr) {
		return this->get_parent()->get_recursive(name, context);
	} else {
		return nullptr;
	}
}
bool ehobj_t::inherited_has(const std::string &key) const {
	if(this->has(key)) {
		return true;
	}
	for(auto &i : super) {
		if(i->get<Object>()->inherited_has(key)) {
			return true;
		}
	}
	return false;
}
bool ehobj_t::inherits(ehval_p obj) {
	for(auto &i : super) {
		if(i == obj || i->get<Object>()->inherits(obj)) {
			return true;
		}
	}
	return false;
}

ehobj_t *ehobj_t::get_parent() const {
	if(this->parent.null()) {
		return nullptr;
	} else {
		return this->parent->get<Object>();
	}
}
std::set<std::string> ehobj_t::member_set() {
	std::set<std::string> out;
	for(auto &i : this->members) {
		out.insert(i.first);
	}
	for(auto &super_class : super) {
		auto member_set = super_class->get<Object>()->member_set();
		out.insert(member_set.begin(), member_set.end());
	}
	return out;
}
ehmember_p ehobj_t::inherited_get(const std::string &key) {
	if(this->has(key)) {
		return this->get_known(key);
	}
	for(std::list<ehval_p>::const_iterator i = super.begin(), end = super.end(); i != end; i++) {
		ehmember_p result = (*i)->get<Object>()->inherited_get(key);
		if(!ehmember_p::null(result)) {
			return result;
		}
	}
	return nullptr;
}
bool ehobj_t::context_compare(const ehcontext_t &key, class EHI *ehi) const {
	// in global context, we never have access to private stuff
	if(key.object.null()) {
		return false;
	} else if(key.scope->get<Object>() == this) {
		return true;
	} else if(key.object->is_a<Object>()) {
		// this may fail when the key.object is not an Object (i.e., )
		ehobj_t *key_obj = key.object->get<Object>();
		return (type_id == key_obj->type_id) || this->context_compare(key_obj->parent, ehi);
	} else {
		const unsigned int key_id = ehi->get_parent()->repo.get_type_id(key.object);
		return type_id == key_id;
	}
}
void ehobj_t::register_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, class EHInterpreter *interpreter_parent) {
	ehval_p func = interpreter_parent->make_method(method);
	this->register_value(name, func, attributes);
}
void ehobj_t::register_value(const std::string &name, ehval_p value, const attributes_t attributes) {
	ehmember_p member;
	member->attribute = attributes;
	member->value = value;
	this->insert(name, member);
}
int ehobj_t::register_member_class(const char *name, const ehobj_t::initializer init_func, const attributes_t attributes, class EHInterpreter *interpreter_parent) {
	ehobj_t *newclass = new ehobj_t();
	ehval_p new_value = Object::make(newclass, interpreter_parent);

	// register class
	newclass->type_id = interpreter_parent->repo.register_class(name, new_value);

	// inherit from Object, except in Object itself
	newclass->inherit(interpreter_parent->base_object);
	newclass->parent = interpreter_parent->global_object;

	init_func(newclass, interpreter_parent);
	// inherit from Object, except in Object itself
	ehmember_p member = ehmember_t::make(attributes, new_value);
	this->insert(name, member);
	return newclass->type_id;
}

ehobj_t::~ehobj_t() {
	// Commenting out for now until I figure out how to get it working.
	//ehi->call_method_obj(this, "finalize", 0, nullptr, nullptr);
}

// eh_exception

eh_exception::~eh_exception() throw() {}
