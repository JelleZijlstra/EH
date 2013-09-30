#include "Class.hpp"
#include "SuperClass.hpp"

EH_INITIALIZER(Class) {
    REGISTER_METHOD(Class, inherit);
    REGISTER_METHOD_RENAME(Class, operator_colon, "operator()");
    REGISTER_METHOD(Class, new);
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
    obj->assert_type<Class>("Class.inherit", ehi);
    args->assert_type<Class>("Class.inherit", ehi);
    obj->get<Class>()->inherit(args);
    return SuperClass::make(args, ehi->get_parent());
}

EH_METHOD(Class, new) {
    return ehi->call_method(obj, "operator()", args, obj);
}
