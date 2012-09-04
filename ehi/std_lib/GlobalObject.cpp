#include "GlobalObject.h"

// type for the global object (currently empty; ultimately library methods should go here)
START_EHLC(GlobalObject)
EHLC_ENTRY(GlobalObject, toString)
END_EHLC()

EH_METHOD(GlobalObject, toString) {
	return ehretval_t::make_string(strdup("(global execution context)"));
}
