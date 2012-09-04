/*
 * Object class
 */
#include "std_lib_includes.h"

EH_METHOD(Object, new);
EH_METHOD(Object, inherit);
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

EXTERN_EHLC(Object)
