#ifndef EH_CLASS_H_
#define EH_CLASS_H_

#include "std_lib_includes.hpp"

EH_CLASS(Class) {
public:
    typedef ehclass_t *const type;
    type value;

    Class(type val) : value(val) {}

    virtual std::list<ehval_p> children() const override {
        std::list<ehval_p> out;
        for(auto &kv : value->members) {
            out.push_back(kv.second->value);
        }
        for(auto &kv : value->instance_members) {
            out.push_back(kv.second->value);
        }
        out.push_back(value->parent);
        for(auto &i : value->super) {
            out.push_back(i);
        }
        assert(out.size() == value->members.size() + value->instance_members.size() + 1 + value->super.size());
        return out;
    }

    virtual unsigned int get_type_id(const class EHInterpreter *parent) override {
        // type of a Class instance is Class
        // TODO: metaclasses? Some other smartness?
        return parent->repo.get_primitive_id<Class>();
    }

    virtual ~Class() {
        delete value;
    }

    virtual bool belongs_in_gc() const {
        return true;
    }

    virtual bool can_create_property() {
        // direct property creation is allowed only on a class object
        return true;
    }

    virtual ehmember_p get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi);

    virtual void set_member_directly(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);

    virtual void set_instance_member_directly(const char *name, ehmember_p new_value, ehcontext_t context, class EHI *ehi) {
        value->instance_members[name] = new_value;
    }

    virtual void printvar(printvar_set &set, int level, EHI *ehi) override;

    virtual ehval_p get_parent_scope();

    static ehval_p make(ehclass_t *obj, EHInterpreter *parent);

    virtual ehmember_p get_instance_member_current_object(const char *name, ehcontext_t context, class EHI *ehi) {
        return value->get_instance_member_current_object(name, context, ehi);
    }

    virtual const std::list<ehval_w> get_super_classes() {
        return value->super;
    }
    virtual bool has_instance_members() const {
        return true;
    }
    virtual void inherit(ehval_p cls) {
        value->inherit(cls);
    }
};

EH_METHOD(Class, new);
EH_METHOD(Class, operator_colon);
EH_METHOD(Class, inherit);
EH_METHOD(Class, toString);

EH_INITIALIZER(Class);

#endif /* EH_CLASS_H_ */
