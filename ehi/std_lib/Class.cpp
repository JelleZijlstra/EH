#include "std_lib_includes.hpp"
#include "Function.hpp"
#include "Object.hpp"

/*
 * ehclass_t
 */

void ehclass_t::inherit(ehval_p superclass) {
    assert(superclass->is_a<Class>());
    super.push_front(superclass);
}

bool ehclass_t::inherits(ehval_p superclass) {
    for(ehval_p cls : super) {
        if(cls == superclass || cls->inherits(superclass)) {
            return true;
        }
    }
    return false;
}

ehmember_p ehclass_t::get_instance_member_current_object(const char *name, ehcontext_t context, EHInterpreter *) {
    if(instance_members.count(name) != 0) {
        return instance_members[name];
    } else {
        return nullptr;
    }
}

void ehclass_t::register_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, EHInterpreter *interpreter_parent) {
    ehval_p func = Function::make(new Function::t(method), interpreter_parent);
    ehmember_p member(attributes, func);
    instance_members[name] = member;
}

void ehclass_t::printvar(printvar_set &set, int level, EHI *ehi) {
    void *ptr = static_cast<void *>(this);
    if(set.count(ptr) == 0) {
        set.insert(ptr);
        std::cout << "@class " << ehi->get_parent()->repo.get_name(type_id) << " [" << std::endl;
        for(auto &i : members) {
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

std::list<ehval_p> ehclass_t::children() const {
    std::list<ehval_p> out;
    for(auto &kv : members) {
        out.push_back(kv.second->value);
    }
    for(auto &kv : instance_members) {
        out.push_back(kv.second->value);
    }
    out.push_back(parent);
    for(auto &i : super) {
        out.push_back(i);
    }
    assert(out.size() == members.size() + instance_members.size() + 1 + super.size());
    return out;
}


/*
 * Class
 */

void Class::printvar(printvar_set &set, int level, EHI *ehi) {
    value->printvar(set, level, ehi);
}

ehval_p Class::get_parent_scope() {
    return value->parent;
}

ehmember_p Class::get_property_current_object(const char *name, ehcontext_t context, class EHInterpreter *interpreter_parent) {
    if(value->members.count(name) != 0) {
        return value->members[name];
    }
    for(ehval_p superclass : value->super) {
        ehmember_p member = superclass->get_property_current_object(name, context, interpreter_parent);
        if(!member.null()) {
            return member;
        }
    }
    return nullptr;
}

void Class::set_member_directly(const char *name, ehmember_p member, ehcontext_t context, class EHI *ehi) {
    value->insert(name, member);
}

bool Class::can_access_private(ehcontext_t context, class EHInterpreter *interpreter_parent) {
    if(context.object == this || context.object->inherits(this)) {
        return true;
    } else {
        ehval_p type_object = context.object->get_type_object(interpreter_parent);
        if(type_object == this || type_object->inherits(this)) {
            return true;
        } else {
            return false;
        }
    }
}

ehval_p Class::make(ehclass_t *obj, EHInterpreter *parent) {
    ehval_p out = parent->allocate<Class>(obj);
    // dirty trick: inherit from Class unless it's not set yet
    // if(!parent->class_object.null() && !parent->class_object->is_a<Null>()) {
    //     obj->inherit(parent->class_object);
    // }
    return out;
}


std::set<std::string> Class::instance_member_set(const EHInterpreter *interpreter_parent) {
    std::set<std::string> members;
    for(auto &pair : value->instance_members) {
        members.insert(pair.first);
    }
    for(ehval_p superclass : value->super) {
        auto superset = superclass->instance_member_set(interpreter_parent);
        members.insert(superset.begin(), superset.end());
    }
    return members;
}

/*
 * User-visible methods
 */

EH_INITIALIZER(Class) {
    REGISTER_METHOD(Class, inherit);
    REGISTER_METHOD_RENAME(Class, operator_colon, "operator()");
    REGISTER_METHOD(Class, new);
    REGISTER_METHOD(Class, toString);
    REGISTER_METHOD(Class, typeId);
}

EH_METHOD(Class, operator_colon) {
    obj->assert_type<Class>("Class.operator()", ehi);
    ehobj_t *new_obj = new ehobj_t();
    new_obj->cls = obj;
    ehval_p ret = Object::make(new_obj, ehi->get_parent());
    ehi->call_method(ret, "initialize", args, obj);
    return ret;
}

EH_METHOD(Class, inherit) {
    if(!obj->has_instance_members()) {
        throw_TypeError("only objects with instance members can inherit other classes", obj, ehi);
    } else if(!args->has_instance_members()) {
        throw_TypeError("inherited class must have instance members", args, ehi);
    }
    obj->inherit(args);
    return nullptr;
}

EH_METHOD(Class, new) {
    return ehi->call_method(obj, "operator()", args, obj);
}

EH_METHOD(Class, toString) {
    obj->assert_type<Class>("Class.toString", ehi);
    const unsigned int type_id = obj->get<Class>()->type_id;
    const std::string name = ehi->get_parent()->repo.get_name(type_id);
    return String::make(strdup(("(class <" + name + ">)").c_str()));
}

EH_METHOD(Class, typeId) {
    obj->assert_type<Class>("Class.typeId", ehi);
    return Integer::make(obj->get<Class>()->type_id);
}
