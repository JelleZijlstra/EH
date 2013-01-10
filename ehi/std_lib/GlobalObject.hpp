#ifndef EH_GLOBAL_OBJECT_H_
#define EH_GLOBAL_OBJECT_H_
/*
 * GlobalObject
 */
#include "std_lib_includes.hpp"

EH_CLASS(GlobalObject) {
public:
	typedef void *type;
	type value;

	virtual bool belongs_in_gc() const {
		return false;
	}

	GlobalObject() {}

	static ehval_p make() {
		return new GlobalObject();
	}
};

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

EH_INITIALIZER(GlobalObject);

#endif /* EH_GLOBAL_OBJECT_H_ */
