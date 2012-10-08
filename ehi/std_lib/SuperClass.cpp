#include "SuperClass.h"

EH_INITIALIZER(SuperClass) {
	REGISTER_METHOD(SuperClass, toString);
}

EH_METHOD(SuperClass, toString) {
	return ehretval_t::make_string(strdup("(inherited class)"));
}
