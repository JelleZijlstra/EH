#include "Bool.h"

START_EHLC(Bool)
EHLC_ENTRY(Bool, initialize)
EHLC_ENTRY(Bool, toString)
EHLC_ENTRY(Bool, toBool)
EHLC_ENTRY(Bool, toInt)
EHLC_ENTRY_RENAME(Bool, operator_bang, "operator!")
END_EHLC()

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
