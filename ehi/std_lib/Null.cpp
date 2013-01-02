#include "Null.hpp"

ehval_p Null::null_obj = new Null();

EH_INITIALIZER(Null) {
	REGISTER_METHOD(Null, initialize);
	REGISTER_METHOD(Null, toString);
	REGISTER_METHOD(Null, toBool);
}

EH_METHOD(Null, initialize) {
	return nullptr;
}
EH_METHOD(Null, toString) {
	ASSERT_NULL_AND_TYPE(Null, "Null.toString");
	return String::make(strdup(""));
}
EH_METHOD(Null, toBool) {
	ASSERT_NULL_AND_TYPE(Null, "Null.toBool");
	return Bool::make(false);
}
