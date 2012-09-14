#include "Array.h"

START_EHLC(Array)
EHLC_ENTRY(Array, initialize)
EHLC_ENTRY(Array, has)
EHLC_ENTRY(Array, length)
EHLC_ENTRY_RENAME(Array, operator_arrow, "operator->")
EHLC_ENTRY_RENAME(Array, operator_arrow_equals, "operator->=")
EHLC_ENTRY(Array, toArray)
EHLC_ENTRY(Array, toTuple)
END_EHLC()

EH_METHOD(Array, initialize) {
	return ehi->to_array(args, obj);
}
EH_METHOD(Array, length) {
	ASSERT_NULL_AND_TYPE(array_e, "Array.length");
	return ehretval_t::make_int(obj->get_arrayval()->size());
}
EH_METHOD(Array, has) {
	ASSERT_OBJ_TYPE(array_e, "Array.length");
	return ehretval_t::make_bool(obj->get_arrayval()->has(args));
}
EH_METHOD(Array, operator_arrow) {
	ASSERT_OBJ_TYPE(array_e, "Array.operator->");
	if(args->type() != int_e && args->type() != string_e) {
		throw_TypeError("Invalid type for argument to Array.operator-> (expected String or Integer)", args->type(), ehi);
	}
	eharray_t *arr = obj->get_arrayval();
	if(arr->has(args)) {
		return arr->operator[](args);
	} else {
		return NULL;
	}
}
EH_METHOD(Array, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, array_e, "Array.operator->=");
	ehretval_p index = args->get_tupleval()->get(0);
	if(index->type() != int_e && index->type() != string_e) {
		throw_TypeError("Invalid type for argument to Array.operator->= (expected String or Integer)", index->type(), ehi);
	}
	obj->get_arrayval()->operator[](index) = args->get_tupleval()->get(1);
	return args->get_tupleval()->get(1);
}
EH_METHOD(Array, toArray) {
  ASSERT_NULL_AND_TYPE(array_e, "Array.toArray");
  return obj;
}
EH_METHOD(Array, toTuple) {
  ASSERT_NULL_AND_TYPE(array_e, "Array.toTuple");
  eharray_t *arr = obj->get_arrayval();
  int length = arr->size();
  ehretval_a values(length);
  // We'll say that output order is unspecified
  int i = 0;
  ARRAY_FOR_EACH_INT(arr, member) {
  	values[i++] = member->second;
  }
  ARRAY_FOR_EACH_STRING(arr, member) {
  	values[i++] = member->second;
  }
  return ehi->make_tuple(new ehtuple_t(length, values));
}
