/*
 * FixedArray
 * Provides contiguous arrays of fixed size.
 */
#include "FixedArray.hpp"

EH_INITIALIZER(FixedArray) {
	REGISTER_METHOD(FixedArray, initialize);
	REGISTER_METHOD_RENAME(FixedArray, operator_arrow, "operator->");
	REGISTER_METHOD_RENAME(FixedArray, operator_arrow_equals, "operator->=");
	REGISTER_METHOD(FixedArray, size);
}

/*
 * @description Initializer. Creates an array of the given size.
 * @argument Size of new array
 * @returns N/A
 */
EH_METHOD(FixedArray, initialize) {
	args->assert_type<Integer>("FixedArray.initialize", ehi);
	const int size = args->get<Integer>();
	if(size < 1) {
		throw_ArgumentError("Size must be positive", "FixedArray.initialize", args, ehi);
	}
	return FixedArray::make(args->get<Integer>(), ehi->get_parent());
}

/*
 * @description Access an element in the array.
 * @argument Index in the array
 * @returns Element at the given index
 */
EH_METHOD(FixedArray, operator_arrow) {
	args->assert_type<Integer>("FixedArray.operator->", ehi);
	ASSERT_RESOURCE(FixedArray, "FixedArray.operator->");
	const int index = args->get<Integer>();
	if(index < 0 || index >= (int) data->size()) {
		throw_ArgumentError("index out of range", "FixedArray.operator->", args, ehi);
	}
	return data->get(index);
}

/*
 * @description Set an element in the array.
 * @argument Tuple of size 2: index in the array and value to set
 * @returns Value set
 */
EH_METHOD(FixedArray, operator_arrow_equals) {
	ASSERT_NARGS(2, "FixedArray.operator->=");
	const Tuple::t *tuple = args->get<Tuple>();
	ehval_p index_val = tuple->get(0);
	index_val->assert_type<Integer>("FixedArray.operator->=", ehi);
	ehval_p value = tuple->get(1);
	ASSERT_RESOURCE(FixedArray, "FixedArray.operator->=");

	const int index = index_val->get<Integer>();
	if(index < 0 || index >= (int) data->size()) {
		throw_ArgumentError("index out of range", "FixedArray.operator->", args, ehi);
	}
	data->set(index, value);
	return value;
}

/*
 * @description Report the size of the array
 * @argument None
 * @returns Integer
 */
EH_METHOD(FixedArray, size) {
	ASSERT_NULL("FixedArray.size");
	ASSERT_RESOURCE(FixedArray, "FixedArray.size");
	return Integer::make(data->size());
}
