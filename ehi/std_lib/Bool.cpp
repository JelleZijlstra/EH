/*
 * Bool
 * Represent boolean values. This class has two possible values: true and
 * false. I should look into making it an Enum.
 */

#include "Bool.h"

EH_INITIALIZER(Bool) {
	REGISTER_METHOD(Bool, initialize);
	REGISTER_METHOD(Bool, toString);
	REGISTER_METHOD(Bool, toBool);
	REGISTER_METHOD(Bool, toInt);
	REGISTER_METHOD_RENAME(Bool, operator_bang, "operator!");
}

/*
 * @description Initializer. Calls the toBool method on its argument.
 * @argument Value to convert to a Bool.
 * @returns N/A
 */
EH_METHOD(Bool, initialize) {
	return ehi->to_bool(args, obj);
}

/*
 * @description Converts a bool to a string, either "true" or "false".
 * @argument None
 * @returns String
 */
EH_METHOD(Bool, toString) {
	ASSERT_NULL_AND_TYPE(bool_e, "Bool.toString");
	char *str;
	if(obj->get_boolval()) {
		str = strdup("true");
	} else {
		str = strdup("false");
	}
	return ehretval_t::make_string(str);
}

/*
 * @description Does nothing.
 * @argument None
 * @returns The bool itself.
 */
EH_METHOD(Bool, toBool) {
	ASSERT_NULL_AND_TYPE(bool_e, "Bool.toBool");
	return obj;
}

/*
 * @description Converts the bool to an Integer (1 for true, 0 for false)
 * @argument None
 * @returns Integer
 */
EH_METHOD(Bool, toInt) {
	ASSERT_NULL_AND_TYPE(bool_e, "Bool.toInt");
	if(obj->get_boolval()) {
		return ehretval_t::make_int(1);
	} else {
		return ehretval_t::make_int(0);
	}
}

/*
 * @description Inverts the value of the bool (true for false, false for true).
 * @argument None
 * @returns Bool
 */
EH_METHOD(Bool, operator_bang) {
  ASSERT_NULL_AND_TYPE(bool_e, "Bool.operator!");
  return ehretval_t::make_bool(!obj->get_boolval());
}
