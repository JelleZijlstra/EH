#include "FixedArray.hpp"

EH_INITIALIZER(FixedArray) {
	REGISTER_METHOD(FixedArray, initialize);
	REGISTER_METHOD_RENAME(FixedArray, operator_arrow, "operator->");
	REGISTER_METHOD_RENAME(FixedArray, operator_arrow_equals, "operator->=");
	REGISTER_METHOD(FixedArray, size);
}

EH_METHOD(FixedArray, initialize) {
	ASSERT_TYPE(args, int_e, "FixedArray.initialize");
	FixedArray *fa = new FixedArray(args->get_intval());
	return ehretval_t::make_resource(obj->get_full_type(), static_cast<LibraryBaseClass *>(fa));
}

EH_METHOD(FixedArray, operator_arrow) {
	ASSERT_TYPE(args, int_e, "FixedArray.operator->");
	ASSERT_RESOURCE(FixedArray, "FixedArray.operator->");
	const int index = args->get_intval();
	if(index < 0 || index >= (int) data->size()) {
		throw_ArgumentError("index out of range", "FixedArray.operator->", args, ehi);
	}
	return data->get(index);
}

EH_METHOD(FixedArray, operator_arrow_equals) {
	ASSERT_NARGS(2, "FixedArray.operator->=");
	const ehtuple_t *tuple = args->get_tupleval();
	ehretval_p index_val = tuple->get(0);
	ASSERT_TYPE(index_val, int_e, "FixedArray.operator->=");
	ehretval_p value = tuple->get(1);
	ASSERT_RESOURCE(FixedArray, "FixedArray.operator->=");

	const int index = index_val->get_intval();
	if(index < 0 || index >= (int) data->size()) {
		throw_ArgumentError("index out of range", "FixedArray.operator->", args, ehi);
	}
	data->set(index, value);
	return value;
}

EH_METHOD(FixedArray, size) {
	ASSERT_NULL("FixedArray.size");
	ASSERT_RESOURCE(FixedArray, "FixedArray.size");
	return ehretval_t::make_int(data->size());	
}
