#include <set>
#include <stdio.h>
#include <string.h>

#include "eh.hpp"
#include "eh_libclasses.hpp"
#include "std_lib/Function.hpp"
#include "std_lib/Binding.hpp"
#include "std_lib/Enum.hpp"
#include "std_lib/MiscellaneousError.hpp"
#include "std_lib/Null.hpp"
#include "std_lib/Object.hpp"
#include "std_lib/ConstError.hpp"
#include "std_lib/VisibilityError.hpp"
#include "std_lib/NameError.hpp"

void check_static_attribute(const char *name, ehmember_p member, ehcontext_t context, EHI * ehi) {
	if(member->isstatic()) {
		throw_MiscellaneousError("static modifier is meaningless outside of class context", ehi);
	}
}

/*
 * ehval_t
 */
unsigned int ehval_t::get_type_id(const class EHInterpreter *parent) {
	return parent->repo.get_type_id(this);
}
ehval_p ehval_t::get_type_object(const class EHInterpreter *parent) {
	const unsigned int type_id = get_type_id(parent);
	return parent->repo.get_object(type_id);
}

void ehval_t::check_can_set_member(ehmember_p current_member, const char *name, ehcontext_t context, EHI *ehi) {
    if(!current_member.null()) {
        if(current_member->isconst()) {
            throw_ConstError(this, name, ehi);
        } else if(current_member->attribute.visibility == private_e && !can_access_private(context, ehi)) {
            throw_VisibilityError(this, name, ehi);
        }
    }
}

void ehval_t::printvar(printvar_set &set, int level, class EHI *ehi) {
	const unsigned int type_id = get_type_id(ehi->get_parent());
	auto name = ehi->get_parent()->repo.get_name(type_id);
	std::cout << "@value <" << name << ">" << std::endl;
}


void ehval_t::set_member(const char *name, ehmember_p member, ehcontext_t context, EHI *ehi) {
	check_static_attribute(name, member, context, ehi);
    // if a property with this name already exists, confirm that it is not const or inaccessible
    ehmember_p current_member = get_property_current_object(name, context, ehi);
    check_can_set_member(current_member, name, context, ehi);
    set_member_directly(name, member, context, ehi);
}

ehmember_p ehval_t::set_property(const char *name, ehval_p new_value, ehcontext_t context, class EHI *ehi) {
	ehmember_p current_member = get_property_current_object(name, context, ehi);
	if(current_member.null()) {
		// now check the type object
		ehval_p type = get_type_object(ehi->get_parent());
		current_member = type->get_instance_member(name, context, ehi);
	}
	check_can_set_member(current_member, name, context, ehi);
	attributes_t attributes = current_member.null() ? attributes_t() : current_member->attribute;
    // set in this object
    ehmember_p new_member = ehmember_t::make(attributes, new_value);
    set_member_directly(name, new_member, context, ehi);
    return new_member;
}

void ehval_t::set_member_directly(const char *name, ehmember_p value, ehcontext_t context, EHI *ehi) {
	throw_TypeError("object does not allow member assignment", this, ehi);
}

void ehval_t::set_instance_member(const char *name, ehmember_p member, ehcontext_t context, EHI *ehi) {
	check_static_attribute(name, member, context, ehi);
    // if a property with this name already exists, confirm that it is not const or inaccessible
    ehmember_p current_member = get_instance_member_current_object(name, context, ehi);
    check_can_set_member(current_member, name, context, ehi);
    set_instance_member_directly(name, member, context, ehi);
}
ehmember_p ehval_t::set_instance_property(const char *name, ehval_p new_value, ehcontext_t context, class EHI *ehi) {
	ehmember_p current_member = get_instance_member(name, context, ehi);
	check_can_set_member(current_member, name, context, ehi);
	attributes_t attributes = current_member.null() ? attributes_t() : current_member->attribute;
    // set in this object
    ehmember_p new_member = ehmember_t::make(attributes, new_value);
    set_instance_member_directly(name, new_member, context, ehi);
    return new_member;
}

ehmember_p ehval_t::get_instance_member(const char *name, ehcontext_t context, class EHI *ehi, bool include_object) {
    ehmember_p my_member = get_instance_member_current_object(name, context, ehi);
    if(!my_member.null()) {
        return my_member;
    }
    for(auto superclass : get_super_classes()) {
        ehmember_p super_member = superclass->get_instance_member(name, context, ehi, false);
        if(!super_member.null()) {
            return super_member;
        }
    }
    if(include_object) {
        ehval_p object_class = ehi->get_parent()->repo.get_primitive_class<Object>();
        return object_class->get_instance_member(name, context, ehi, false);
    }
    return nullptr;
}

void ehval_t::set_instance_member_directly(const char *name, ehmember_p value, ehcontext_t context, EHI *ehi) {
	throw_TypeError("object does not allow instance member assignment", this, ehi);
}
ehmember_p ehval_t::get_instance_member_current_object(const char *name, ehcontext_t context, class EHI *ehi) {
	// by default, no properties on the actual object
	return nullptr;
}

ehmember_p ehval_t::get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi) {
	// by default, no properties on the actual object
	return nullptr;
}
ehmember_p ehval_t::get_property_no_binding(const char *name, ehcontext_t context, EHI *ehi) {
	ehmember_p member = get_property_current_object(name, context, ehi);
	if(member.null()) {
		// now check the type object
		ehval_p type = get_type_object(ehi->get_parent());
		member = type->get_instance_member(name, context, ehi);
	}
	if(member.null()) {
		throw_NameError(this, name, ehi);
	} else if(member->attribute.visibility == private_e && !can_access_private(context, ehi)) {
		throw_VisibilityError(this, name, ehi);
	} else {
		return member;
	}
}
// get a property of the given name from the base_var object, creating a binding if necessary
ehval_p ehval_t::get_property(const char *name, ehcontext_t context, EHI *ehi) {
	ehval_p out = get_property_no_binding(name, context, ehi)->value;
	if(out->is_a<Function>()) {
		return Binding::make(this, out, ehi->get_parent());
	} else {
		return out;
	}
}
ehmember_p ehval_t::get_property_up_scope_chain(const char *name, ehcontext_t context, class EHI *ehi) {
	ehmember_p member = get_property_current_object(name, context, ehi);
	if(member.null()) {
		ehval_p parent = get_parent_scope();
		if(!parent.null() && !parent->is_a<Null>()) {
			member = parent->get_property_up_scope_chain(name, context, ehi);
		}
	}
	return member;
}

bool ehval_t::can_access_private(ehcontext_t context, EHI *ehi) {
	return (context.scope == this) || (context.object == this) || get_type_object(ehi->get_parent())->can_access_private(context, ehi);
}

std::set<std::string> ehval_t::member_set(const EHInterpreter *interpreter_parent) {
	return get_type_object(interpreter_parent)->instance_member_set(interpreter_parent);
}

/*
 * eh_exception
 */
eh_exception::~eh_exception() noexcept {}
