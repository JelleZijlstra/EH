/*
 * Float class
 */
#include "std_lib_includes.h"

EH_METHOD(Float, initialize);
EH_METHOD(Float, operator_plus);
EH_METHOD(Float, operator_minus);
EH_METHOD(Float, operator_times);
EH_METHOD(Float, operator_divide);
EH_METHOD(Float, operator_uminus);
EH_METHOD(Float, compare);
EH_METHOD(Float, abs);
EH_METHOD(Float, toString);
EH_METHOD(Float, toInt);
EH_METHOD(Float, toBool);
EH_METHOD(Float, toFloat);
EH_METHOD(Float, sqrt);

EXTERN_EHLC(Float)
