#include "SuperClass.h"

START_EHLC(SuperClass)
EHLC_ENTRY(SuperClass, toString)
END_EHLC()

EH_METHOD(SuperClass, toString) {
	return ehretval_t::make_string(strdup("(inherited class)"));
}
