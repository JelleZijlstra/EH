#include "Tuple.h"

START_EHLC(Tuple)
EHLC_ENTRY(Tuple, initialize)
EHLC_ENTRY_RENAME(Tuple, operator_arrow, "operator->")
EHLC_ENTRY(Tuple, length)
EHLC_ENTRY(Tuple, toTuple)
END_EHLC()

EH_METHOD(Tuple, initialize) {
	return ehi->to_tuple(args, obj);
}
EH_METHOD(Tuple, operator_arrow) {
	ASSERT_OBJ_TYPE(tuple_e, "Tuple.operator->");
	ASSERT_TYPE(args, int_e, "Tuple.operator->");
	int index = args->get_intval();
	if(index < 0 || index >= obj->get_tupleval()->size()) {
    	throw_ArgumentError_out_of_range("Tuple.operator->", args, ehi);
	}
  	return obj->get_tupleval()->get(index);
}
EH_METHOD(Tuple, length) {
  	ASSERT_NULL_AND_TYPE(tuple_e, "Tuple.length");
  	return ehretval_t::make_int(obj->get_tupleval()->size());
}
EH_METHOD(Tuple, toTuple) {
	ASSERT_OBJ_TYPE(tuple_e, "Tuple.toTuple");
	return obj;
}
