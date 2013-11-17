#include <set>

#include "Array.hpp"
#include "Function.hpp"
#include "Object.hpp"

/*
 * ehobj_t
 */

void ehobj_t::register_static_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, EHInterpreter *interpreter_parent) {
    ehval_p func = Function::make(new Function::t(method), interpreter_parent);
    this->register_value(name, func, attributes);
}

void ehobj_t::register_value(const std::string &name, ehval_p value, const attributes_t attributes) {
    ehmember_p member(attributes, value);
    this->insert(name, member);
}

unsigned int ehobj_t::register_member_class(const char *name, const initializer init_func, const attributes_t attributes, EHInterpreter *interpreter_parent) {
    ehclass_t *newclass = new ehclass_t(name);
    ehval_p new_value = Class::make(newclass, interpreter_parent);

    // register class
    newclass->type_id = interpreter_parent->repo.register_class(name, new_value);

    newclass->parent = interpreter_parent->global_object;

    init_func(newclass, interpreter_parent);

    ehmember_p member = ehmember_t::make(attributes, new_value);
    this->insert(name, member);
    return newclass->type_id;
}

/*
 * Object
 */
ehval_p Object::make(ehobj_t *obj, EHInterpreter *parent) {
    return parent->allocate<Object>(obj);
}

void Object::printvar(printvar_set &set, int level, EHI *ehi) {
    void *ptr = static_cast<void *>(value);
    if(set.count(ptr) == 0) {
        set.insert(ptr);
        const std::string name = ehi->get_parent()->repo.get_name(get_type_id(ehi->get_parent()));
        std::cout << "@object <" << name << "> [" << std::endl;
        for(auto &i : value->members) {
            add_tabs(std::cout, level + 1);
            std::cout << i.first << " <";
            const attributes_t attribs = i.second->attribute;
            std::cout << (attribs.visibility == public_e ? "public" : "private") << ",";
            std::cout << (attribs.isconst == const_e ? "constant" : "non-constant") << ">: ";
            i.second->value->printvar(set, level + 1, ehi);
        }
        add_tabs(std::cout, level);
        std::cout << "]" << std::endl;
    } else {
        std::cout << "(recursion)" << std::endl;
    }
}

void Object::set_member_directly(const char *name, ehmember_p member, ehcontext_t context, class EHI *ehi) {
    value->insert(name, member);
}

ehmember_p Object::get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi) {
    if(value->members.count(name) != 0) {
        return value->members[name];
    } else {
        return nullptr;
    }
}

unsigned int Object::get_type_id(const class EHInterpreter *parent) {
	return value->cls->get<Class>()->type_id;
}

std::set<std::string> Object::member_set(const EHInterpreter *interpreter_parent) {
	auto members = get_type_object(interpreter_parent)->instance_member_set(interpreter_parent);
	for(auto &pair : value->members) {
		members.insert(pair.first);
	}
	// always add Object members
	for(auto &pair : interpreter_parent->base_object->get<Class>()->instance_members) {
		members.insert(pair.first);
	}
	return members;
}


/*
 * User-visible methods
 */

EH_INITIALIZER(Object) {
	REGISTER_METHOD(Object, initialize);
	REGISTER_METHOD(Object, toString);
	REGISTER_METHOD(Object, finalize);
	REGISTER_METHOD(Object, isA);
	REGISTER_METHOD_RENAME(Object, operator_compare, "operator<=>");
	REGISTER_METHOD(Object, compare);
	REGISTER_METHOD_RENAME(Object, operator_equals, "operator==");
	REGISTER_METHOD_RENAME(Object, operator_ne, "operator!=");
	REGISTER_METHOD_RENAME(Object, operator_gt, "operator>");
	REGISTER_METHOD_RENAME(Object, operator_gte, "operator>=");
	REGISTER_METHOD_RENAME(Object, operator_lt, "operator<");
	REGISTER_METHOD_RENAME(Object, operator_lte, "operator<=");
	REGISTER_METHOD(Object, type);
	REGISTER_METHOD(Object, typeId);
	REGISTER_METHOD(Object, members);
	REGISTER_METHOD(Object, clone);
}

EH_METHOD(Object, initialize) {
	return nullptr;
}
EH_METHOD(Object, toString) {
	args->assert_type<Null>("Object.toString", ehi);
	const size_t len = sizeof(void *) * 2 + 3;
	char *out = new char[len]();
	snprintf(out, len, "%p", reinterpret_cast<void *>(obj.operator->()));
	return String::make(out);
}
EH_METHOD(Object, finalize) {
	return nullptr;
}
EH_METHOD(Object, isA) {
	args->assert_type<Class>("Object.isA", ehi);
	ehval_p type_object = obj->get_type_object(ehi->get_parent());
	// everything inherits Object
	if(type_object == args || args == ehi->get_parent()->base_object) {
		return Bool::make(true);
	} else {
		return Bool::make(type_object->inherits(args));
	}
}

EH_METHOD(Object, operator_compare) {
	unsigned int lhs_type = obj->get_type_id(ehi->get_parent());
	unsigned int rhs_type = args->get_type_id(ehi->get_parent());
	int comparison = intcmp(lhs_type, rhs_type);
	if(comparison != 0) {
		return Integer::make(comparison);
	} else {
		return ehi->call_method(obj, "compare", args, obj);
	}
}
EH_METHOD(Object, compare) {
	if(!obj->equal_type(args)) {
		throw_TypeError("Arguments to Object.compare must have the same type", args, ehi);
	}
	return Integer::make(obj->naive_compare(args));
}
#define CALL_COMPARE() \
	ehval_p comparison_p = ehi->call_method_typed<Integer>(obj, "operator<=>", args, obj); \
	int comparison = comparison_p->get<Integer>();

EH_METHOD(Object, operator_equals) {
	CALL_COMPARE();
	return Bool::make(comparison == 0);
}
EH_METHOD(Object, operator_ne) {
	CALL_COMPARE();
	return Bool::make(comparison != 0);
}
EH_METHOD(Object, operator_gt) {
	CALL_COMPARE();
	return Bool::make(comparison > 0);
}
EH_METHOD(Object, operator_gte) {
	CALL_COMPARE();
	return Bool::make(comparison >= 0);
}
EH_METHOD(Object, operator_lt) {
	CALL_COMPARE();
	return Bool::make(comparison < 0);
}
EH_METHOD(Object, operator_lte) {
	CALL_COMPARE();
	return Bool::make(comparison <= 0);
}
EH_METHOD(Object, type) {
	unsigned int type_id = obj->get_type_id(ehi->get_parent());
	return ehi->get_parent()->repo.get_object(type_id);
}
EH_METHOD(Object, typeId) {
	return Integer::make(static_cast<Integer::type>(obj->get_type_id(ehi->get_parent())));
}
// return all members in the class
EH_METHOD(Object, members) {
	std::set<std::string> members = obj->member_set(ehi->get_parent());
	ehval_p out = Array::make(ehi->get_parent());
	Array::t *arr = out->get<Array>();
	for(std::set<std::string>::iterator i = members.begin(), end = members.end(); i != end; i++) {
		arr->append(String::make(strdup((*i).c_str())));
	}
	return out;
}
EH_METHOD(Object, clone) {
	obj->assert_type<Object>("Object.clone", ehi);
	ehobj_t *new_obj = new ehobj_t();
	new_obj->cls = obj->get<Object>()->cls;
	for(auto &pair : obj->get<Object>()->members) {
		new_obj->members[pair.first] = pair.second;
	}
	return Object::make(new_obj, ehi->get_parent());
}
