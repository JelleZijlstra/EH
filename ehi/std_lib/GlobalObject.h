/*
 * GlobalObject
 */
#include "std_lib_includes.h"

EH_METHOD(GlobalObject, toString);
EH_METHOD(GlobalObject, getinput);
EH_METHOD(GlobalObject, printvar);
EH_METHOD(GlobalObject, is_null);
EH_METHOD(GlobalObject, is_string);
EH_METHOD(GlobalObject, is_int);
EH_METHOD(GlobalObject, is_bool);
EH_METHOD(GlobalObject, is_array);
EH_METHOD(GlobalObject, is_object);
EH_METHOD(GlobalObject, is_range);
EH_METHOD(GlobalObject, is_float);
EH_METHOD(GlobalObject, get_type);
EH_METHOD(GlobalObject, include);
EH_METHOD(GlobalObject, pow);
EH_METHOD(GlobalObject, log);
EH_METHOD(GlobalObject, eval);
EH_METHOD(GlobalObject, throw);
EH_METHOD(GlobalObject, echo);
EH_METHOD(GlobalObject, put);
EH_METHOD(GlobalObject, collectGarbage);

EXTERN_EHLC(GlobalObject)
