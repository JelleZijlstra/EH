#include "Null.hpp"

EH_INITIALIZER(Null) {
	REGISTER_METHOD(Null, initialize);
	REGISTER_METHOD(Null, toString);
	REGISTER_METHOD(Null, toBool);
}

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
