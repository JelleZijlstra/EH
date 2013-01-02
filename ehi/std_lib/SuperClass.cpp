#include "SuperClass.hpp"

ehval_p SuperClass::make(ehval_p super, EHInterpreter *parent) {
	return parent->allocate<SuperClass>(super);
}

EH_INITIALIZER(SuperClass) {
	REGISTER_METHOD(SuperClass, toString);
}

EH_METHOD(SuperClass, toString) {
	return String::make(strdup("(inherited class)"));
}
