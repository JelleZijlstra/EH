#include "eh.hpp"
#include "std_lib/ConstError.hpp"
#include "std_lib/Enum.hpp"
#include "std_lib/Function.hpp"
#include "std_lib/VisibilityError.hpp"

void ehclass_t::inherit(ehval_p superclass) {
    assert(superclass->is_a<Class>());
    super.push_front(superclass);
}

bool ehclass_t::inherits(ehval_p superclass, EHInterpreter *interpreter_parent) {
    if(!superclass->is_a<Class>()) {
        return false;
    }
    if(superclass == interpreter_parent->base_object) {
        return true;
    }
    for(auto &i : super) {
        if(i == superclass || i->get<Class>()->inherits(superclass, interpreter_parent)) {
            return true;
        }
    }
    return false;
}

ehmember_p ehclass_t::get_instance_member_current_object(const char *name, ehcontext_t context, class EHI *ehi) {
    if(instance_members.count(name) != 0) {
        return instance_members[name];
    } else {
        return nullptr;
    }
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

void Class::printvar(printvar_set &set, int level, EHI *ehi) {
    void *ptr = static_cast<void *>(value);
    if(set.count(ptr) == 0) {
        set.insert(ptr);
        std::cout << "@class " << ehi->get_parent()->repo.get_name(value->type_id) << " [" << std::endl;
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

/*
 * Class
 */

ehval_p Class::make(ehclass_t *obj, EHInterpreter *parent) {
    ehval_p out = parent->allocate<Class>(obj);
    // dirty trick: inherit from Class unless it's not set yet
    if(!parent->class_object.null() && !parent->class_object->is_a<Null>()) {
        obj->inherit(parent->class_object);
    }
    return out;
}

/*
 * ehobj_t
 */
// const methods

// TODO: fix this
// std::set<std::string> ehobj_t::member_set(const EHInterpreter *interpreter_parent) const {
//     std::set<std::string> out;
//     for(auto &i : this->members) {
//         out.insert(i.first);
//     }
//     for(auto &super_class : super) {
//         auto member_set = super_class->get<Object>()->member_set(nullptr);
//         out.insert(member_set.begin(), member_set.end());
//     }
//     if(interpreter_parent != nullptr) {
//         ehobj_t *base_object = interpreter_parent->base_object->get<Object>();
//         auto object_set = base_object->member_set(nullptr);
//         out.insert(object_set.begin(), object_set.end());
//     }
//     return out;
// }

// bool ehobj_t::context_compare(const ehcontext_t &key, const class EHI *ehi) const {
//     // in global context, we never have access to private stuff
//     if(key.object->is_a<Null>()) {
//         return false;
//     } else if(key.scope->get<Object>() == this) {
//         return true;
//     } else if(key.object->is_a<Object>()) {
//         // this may fail when the key.object is not an Object (i.e., )
//         ehobj_t *key_obj = key.object->get<Object>();
//         return (type_id == key_obj->type_id) || this->context_compare(ehcontext_t(key_obj->parent, key_obj->parent), ehi);
//     } else {
//         const unsigned int key_id = ehi->get_parent()->repo.get_type_id(key.object);
//         return type_id == key_id;
//     }
// }

ehval_p Class::get_parent_scope() {
    return value->parent;
}
ehmember_p Class::get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi) {
    // TODO: maybe this should check the instance_members too?
    if(value->members.count(name) != 0) {
        return value->members[name];
    } else {
        return nullptr;
    }
}
void Class::set_member_directly(const char *name, ehmember_p member, ehcontext_t context, class EHI *ehi) {
    value->insert(name, member);
}


void ehclass_t::register_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, EHInterpreter *interpreter_parent) {
    ehval_p func = Function::make(new Function::t(method), interpreter_parent);
    ehmember_p member(attributes, func);
    instance_members[name] = member;
}

void ehobj_t::register_static_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, EHInterpreter *interpreter_parent) {
    ehval_p func = Function::make(new Function::t(method), interpreter_parent);
    this->register_value(name, func, attributes);
}

void ehobj_t::register_value(const std::string &name, ehval_p value, const attributes_t attributes) {
    ehmember_p member(attributes, value);
    this->insert(name, member);
}

unsigned int ehobj_t::register_member_class(const char *name, const initializer init_func, const attributes_t attributes, EHInterpreter *interpreter_parent) {
    ehclass_t *newclass = new ehclass_t();
    ehval_p new_value = Class::make(newclass, interpreter_parent);

    // register class
    newclass->type_id = interpreter_parent->repo.register_class(name, new_value);

    newclass->parent = interpreter_parent->global_object;

    init_func(newclass, interpreter_parent);

    ehmember_p member = ehmember_t::make(attributes, new_value);
    this->insert(name, member);
    return newclass->type_id;
}
