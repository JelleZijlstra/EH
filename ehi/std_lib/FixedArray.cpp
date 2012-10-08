#include "FixedArray.h"

EH_INITIALIZER(FixedArray) {
	REGISTER_METHOD(FixedArray, initialize);
	REGISTER_METHOD_RENAME(FixedArray, operator_arrow, "operator->");
	REGISTER_METHOD_RENAME(FixedArray, operator_arrow_equals, "operator->=");
	REGISTER_METHOD(FixedArray, size);
}

EH_METHOD(FixedArray, initialize) {
	ASSERT_TYPE(args, int_e, "FixedArray.initialize");
	FixedArray *fa = new FixedArray(args->get_intval());
	return ehretval_t::make_resource(static_cast<LibraryBaseClass *>(fa));
}

EH_METHOD(FixedArray, operator_arrow) {
	ASSERT_OBJ_TYPE(resource_e, "FixedArray.operator->");
	ASSERT_TYPE(args, int_e, "FixedArray.operator->");
	const FixedArray *fa = static_cast<FixedArray *>(obj->get_resourceval());
	const int index = args->get_intval();
	if(index < 0 || index >= (int) fa->size()) {
		throw_ArgumentError("index out of range", "FixedArray.operator->", args, ehi);
	}
	return fa->get(index);
}

EH_METHOD(FixedArray, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, resource_e, "FixedArray.operator->=");
	const ehtuple_t *tuple = args->get_tupleval();
	ehretval_p index_val = tuple->get(0);
	ASSERT_TYPE(index_val, int_e, "FixedArray.operator->=");
	ehretval_p value = tuple->get(1);

	FixedArray *fa = static_cast<FixedArray *>(obj->get_resourceval());
	const int index = index_val->get_intval();
	if(index < 0 || index >= (int) fa->size()) {
		throw_ArgumentError("index out of range", "FixedArray.operator->", args, ehi);
	}
	fa->set(index, value);
	return value;
}

EH_METHOD(FixedArray, size) {
	ASSERT_NULL_AND_TYPE(resource_e, "FixedArray.size");
	const FixedArray *fa = static_cast<FixedArray *>(obj->get_resourceval());
	return ehretval_t::make_int(fa->size());	
}
