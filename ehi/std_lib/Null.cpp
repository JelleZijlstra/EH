#include "Null.h"

START_EHLC(Null)
EHLC_ENTRY(Null, initialize)
EHLC_ENTRY(Null, toString)
EHLC_ENTRY(Null, toBool)
END_EHLC()

EH_METHOD(Null, initialize) {
	return NULL;
}
EH_METHOD(Null, toString) {
	ASSERT_NULL_AND_TYPE(null_e, "Null.toString");
	return ehretval_t::make_string(strdup(""));
}
EH_METHOD(Null, toBool) {
	ASSERT_NULL_AND_TYPE(null_e, "Null.toBool");
	return ehretval_t::make_bool(false);
}
