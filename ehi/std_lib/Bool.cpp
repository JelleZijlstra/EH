#include "Bool.h"

EH_INITIALIZER(Bool) {
	REGISTER_METHOD(Bool, initialize);
	REGISTER_METHOD(Bool, toString);
	REGISTER_METHOD(Bool, toBool);
	REGISTER_METHOD(Bool, toInt);
	REGISTER_METHOD_RENAME(Bool, operator_bang, "operator!");
}

EH_METHOD(Bool, initialize) {
	return ehi->to_bool(args, obj);
}
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
EH_METHOD(Bool, toBool) {
	ASSERT_NULL_AND_TYPE(bool_e, "Bool.toBool");
	return obj;
}
EH_METHOD(Bool, toInt) {
	ASSERT_NULL_AND_TYPE(bool_e, "Bool.toInt");
	if(obj->get_boolval()) {
		return ehretval_t::make_int(1);
	} else {
		return ehretval_t::make_int(0);
	}
}
EH_METHOD(Bool, operator_bang) {
  ASSERT_NULL_AND_TYPE(bool_e, "Bool.operator!");
  return ehretval_t::make_bool(!obj->get_boolval());
}
