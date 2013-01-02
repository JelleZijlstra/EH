/*
 * Bool
 * Represent boolean values. This class has two possible values: true and
 * false. I should look into making it an Enum.
 */

#include "Bool.hpp"

EH_INITIALIZER(Bool) {
	REGISTER_METHOD(Bool, initialize);
	REGISTER_METHOD(Bool, toString);
	REGISTER_METHOD(Bool, toBool);
	REGISTER_METHOD(Bool, toInteger);
	REGISTER_METHOD(Bool, compare);
	REGISTER_METHOD_RENAME(Bool, operator_bang, "operator!");
}

/*
 * @description Initializer. Calls the toBool method on its argument.
 * @argument Value to convert to a Bool.
 * @returns N/A
 */
EH_METHOD(Bool, initialize) {
	return ehi->toBool(args, obj);
}

/*
 * @description Compare one bool to another.
 * @argument Value to compare to
 * @return Integer
 */
EH_METHOD(Bool, compare) {
	ASSERT_TYPE(args, Bool, "Bool.compare");
	ASSERT_OBJ_TYPE(Bool, "Bool.compare");
	bool lhs = obj->get<Bool>();
	bool rhs = args->get<Bool>();
	if(lhs == rhs) {
		return Integer::make(0);
	} else if(lhs == false) {
		return Integer::make(-1);
	} else {
		return Integer::make(1);
	}
}

/*
 * @description Converts a bool to a string, either "true" or "false".
 * @argument None
 * @returns String
 */
EH_METHOD(Bool, toString) {
	ASSERT_NULL_AND_TYPE(Bool, "Bool.toString");
	char *str;
	if(obj->get<Bool>()) {
		str = strdup("true");
	} else {
		str = strdup("false");
	}
	return String::make(str);
}

/*
 * @description Does nothing.
 * @argument None
 * @returns The bool itself.
 */
EH_METHOD(Bool, toBool) {
	ASSERT_NULL_AND_TYPE(Bool, "Bool.toBool");
	return obj;
}

/*
 * @description Converts the bool to an Integer (1 for true, 0 for false)
 * @argument None
 * @returns Integer
 */
EH_METHOD(Bool, toInteger) {
	ASSERT_NULL_AND_TYPE(Bool, "Bool.toInteger");
	if(obj->get<Bool>()) {
		return Integer::make(1);
	} else {
		return Integer::make(0);
	}
}

/*
 * @description Inverts the value of the bool (true for false, false for true).
 * @argument None
 * @returns Bool
 */
EH_METHOD(Bool, operator_bang) {
  ASSERT_NULL_AND_TYPE(Bool, "Bool.operator!");
  return Bool::make(!obj->get<Bool>());
}
