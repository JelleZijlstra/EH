/*
* String class
*/
#include "std_lib_includes.h"

EH_METHOD(String, initialize);
EH_METHOD(String, length);
EH_METHOD(String, operator_arrow);
EH_METHOD(String, operator_arrow_equals);
EH_METHOD(String, operator_plus);
EH_METHOD(String, compare);
EH_METHOD(String, operator_equals);
EH_METHOD(String, toString);
EH_METHOD(String, toInt);
EH_METHOD(String, toFloat);
EH_METHOD(String, toBool);
EH_METHOD(String, toRange);
EH_METHOD(String, charAtPosition);

EXTERN_EHLC(String)
