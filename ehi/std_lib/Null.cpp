#include "Null.hpp"

ehval_p Null::null_obj = new Null();

EH_INITIALIZER(Null) {
	REGISTER_METHOD(Null, initialize);
	REGISTER_METHOD(Null, toString);
	REGISTER_METHOD(Null, toBool);
	REGISTER_METHOD(Null, compare);
}

/*
 * @description Initializer.
 * @argument None
 * @returns N/A
 */
EH_METHOD(Null, initialize) {
	return nullptr;
}

/*
 * @description Converts null to a string (the empty string)
 * @argument None
 * @returns The empty string
 */
EH_METHOD(Null, toString) {
	ASSERT_NULL_AND_TYPE(Null, "Null.toString");
	return String::make(strdup(""));
}

/*
 * @description Converts null to a Bool (always false)
 * @argument None
 * @returns False
 */
EH_METHOD(Null, toBool) {
	ASSERT_NULL_AND_TYPE(Null, "Null.toBool");
	return Bool::make(false);
}

/*
 * @description Compares null to itself.
 * @argument Null
 * @returns Always returns true
 */
EH_METHOD(Null, compare) {
	ASSERT_OBJ_TYPE(Null, "Null.compare");
	ASSERT_TYPE(args, Null, "Null.compare");
	return Integer::make(0);
}
