#ifndef EH_GLOBAL_OBJECT_H_
#define EH_GLOBAL_OBJECT_H_
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
EH_METHOD(GlobalObject, throw);
EH_METHOD(GlobalObject, echo);
EH_METHOD(GlobalObject, put);
EH_METHOD(GlobalObject, handleUncaught);
EH_METHOD(GlobalObject, workingDir);
EH_METHOD(GlobalObject, shell);
EH_METHOD(GlobalObject, exit);

EH_INITIALIZER(GlobalObject);
void ehinstance_init_GlobalObject(ehobj_t *obj, EHInterpreter *parent);

#endif /* EH_GLOBAL_OBJECT_H_ */
