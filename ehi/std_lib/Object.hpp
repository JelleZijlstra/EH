/*
 * Object class
 */
#ifndef EH_OBJECT_H_
#define EH_OBJECT_H_

#include "std_lib_includes.hpp"

EH_CLASS(Object) {
public:
    typedef ehobj_t *const type;
    type value;

    Object(ehobj_t *val) : value(val) {}

    virtual ~Object() {
        delete value;
    }

    virtual bool belongs_in_gc() const {
        return true;
    }

    virtual std::list<ehval_p> children() const override {
        std::list<ehval_p> out;
        for(auto &kv : value->members) {
            out.push_back(kv.second->value);
        }
        out.push_back(value->cls);
        assert(out.size() == value->members.size() + 1);
        return out;
    }

    virtual void printvar(printvar_set &set, int level, EHI *ehi) override;

    static ehval_p make(ehobj_t *obj, EHInterpreter *parent);

    virtual unsigned int get_type_id(const class EHInterpreter *parent) override;

    virtual ehval_p get_type_object(const EHInterpreter *parent) override {
        return value->cls;
    }

    virtual ehmember_p get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi);

    virtual void set_member_directly(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);

    virtual bool inherits(ehval_p superclass) override {
        return value->cls->inherits(superclass);
    }

    virtual std::set<std::string> member_set(const EHInterpreter *interpreter_parent) override;
};

EH_METHOD(Object, initialize);
EH_METHOD(Object, toString);
EH_METHOD(Object, finalize);
EH_METHOD(Object, isA);
EH_METHOD(Object, operator_compare);
EH_METHOD(Object, compare);
EH_METHOD(Object, operator_equals);
EH_METHOD(Object, operator_ne);
EH_METHOD(Object, operator_gt);
EH_METHOD(Object, operator_gte);
EH_METHOD(Object, operator_lt);
EH_METHOD(Object, operator_lte);
EH_METHOD(Object, type);
EH_METHOD(Object, typeId);
EH_METHOD(Object, members);
EH_METHOD(Object, clone);

EH_INITIALIZER(Object);

#endif /* EH_OBJECT_H_ */
