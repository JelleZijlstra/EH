/*
 * GlobalObject
 */
#include "std_lib_includes.hpp"

EH_METHOD(GlobalObject, toString);
EH_METHOD(GlobalObject, getinput);
EH_METHOD(GlobalObject, printvar);
EH_METHOD(GlobalObject, include);
EH_METHOD(GlobalObject, pow);
EH_METHOD(GlobalObject, log);
EH_METHOD(GlobalObject, eval);
EH_METHOD(GlobalObject, throw);
EH_METHOD(GlobalObject, echo);
EH_METHOD(GlobalObject, put);
EH_METHOD(GlobalObject, collectGarbage);
EH_METHOD(GlobalObject, handleUncaught);
EH_METHOD(GlobalObject, contextName);
EH_METHOD(GlobalObject, workingDir);

EH_INITIALIZER(GlobalObject);