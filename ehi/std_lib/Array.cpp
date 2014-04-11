/*
 * Array
 * Used as a map from strings and integers to arbitrary values, similar to a PHP
 * array. Array literals can be constructed using the form <tt>[foo, bar]</tt>
 * (using consecutive numeric indices). It is possible to specify an index
 * explicitly, e.g., <tt>["foo" => foo, 42 => bar]</tt>.
 *
 * In the future, I hope to replace this class with something more similar to
 * Python lists and JavaScript arrays, while using the Map class for more
 * general maps.
 */
#include "Array.hpp"

#include "ArgumentError.hpp"
#include "EmptyIterator.hpp"

EH_INITIALIZER(Array) {
	REGISTER_CONSTRUCTOR(Array);
	REGISTER_METHOD(Array, has);
	REGISTER_METHOD(Array, length);
	REGISTER_METHOD_RENAME(Array, operator_arrow, "operator->");
	REGISTER_METHOD_RENAME(Array, operator_arrow_equals, "operator->=");
	REGISTER_METHOD(Array, push);
	REGISTER_METHOD(Array, pop);
	REGISTER_METHOD(Array, toArray);
	REGISTER_METHOD(Array, toTuple);
	REGISTER_METHOD(Array, compare);
	REGISTER_METHOD(Array, getIterator);
	REGISTER_CLASS(Array, Iterator);
}

ehval_p Array::make(EHInterpreter *parent, int size) {
	return parent->allocate<Array>(new t(size));
}

void Array::t::insert(int index, ehval_p val) {
	// Inserts a member into an array.
	assert(this->has(index));
	this->v[index] = val;
}

void Array::t::append(ehval_p val) {
	this->v.push_back(val);
}

ehval_p Array::t::pop() {
	ehval_p val = this->v.back();
	this->v.pop_back();
	return val;
}

int Array::t::compare(Array::t *rhs, ehcontext_t context, EHI *ehi) {
	const size_t lhs_size = this->size();
	const size_t rhs_size = rhs->size();
	if(lhs_size != rhs_size) {
		return intcmp(lhs_size, rhs_size);
	}
	// compare integer keys
	for(size_t i = 0; i < lhs_size; i++) {
		int comparison = ehi->compare(this->v[i], rhs->v[i], context);
		if(comparison != 0) {
			return comparison;
		}
	}
	return 0;
}

/*
 * @description Converts its argument to an array by calling its toArray method.
 * @argument Anything with a toArray method.
 * @returns Array.
 */
EH_METHOD(Array, operator_colon) {
	if(args->is_a<Null>()) {
		return Array::make(ehi->get_parent());
	} else {
		return ehi->call_method_typed<Array>(args, "toArray", nullptr, obj);
	}
}

/*
 * @description Length of an array.
 * @argument None.
 * @returns Number of items in the array.
 */
EH_METHOD(Array, length) {
	ASSERT_NULL_AND_TYPE(Array, "Array.length");
	return Integer::make(static_cast<int>(obj->get<Array>()->size()));
}

/*
 * @description Checks whether a given key exists in the array.
 * @argument String or integer key.
 * @returns True if key exists, false if not.
 */
EH_METHOD(Array, has) {
	ASSERT_OBJ_TYPE(Array, "Array.has");
	ASSERT_TYPE(args, Integer, "Array.has");
	return Bool::make(obj->get<Array>()->has(args->get<Integer>()));
}

/*
 * @description Access the element with the given key
 * @argument String or integer key.
 * @returns Value associated with the key; throws ArgumentError if key does not exist.
 */
EH_METHOD(Array, operator_arrow) {
	ASSERT_OBJ_TYPE(Array, "Array.operator->");
	ASSERT_TYPE(args, Integer, "Array.operator->");
	const int index = args->get<Integer>();
	Array::t *arr = obj->get<Array>();
	if(arr->has(index)) {
		return arr->v[index];
	} else {
		throw_ArgumentError_out_of_range("Array.operator->", args, ehi);
	}
}

/*
 * @description Set the element with the given key
 * @argument Tuple of size 2: key and value to set to
 * @returns Value set
 */
EH_METHOD(Array, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, Array, "Array.operator->=");
	ehval_p index_v = args->get<Tuple>()->get(0);
	ASSERT_TYPE(index_v, Integer, "Array.operator->=");
	int index = index_v->get<Integer>();
	Array::t *data = obj->get<Array>();
	if(!data->has(index)) {
		throw_ArgumentError_out_of_range("Array.operator->=", index_v, ehi);
	}
	ehval_p rvalue = args->get<Tuple>()->get(1);
	data->v[index] = rvalue;
	return rvalue;
}

/*
 * @description Append a new value to the end of the array
 * @argument Value to add
 * @returns The array
 */
EH_METHOD(Array, push) {
	ASSERT_OBJ_TYPE(Array, "Array.push");
	obj->get<Array>()->append(args);
	return obj;
}

/*
 * @description Removes the last value from the array
 * @argument None
 * @returns Value removed
 */
EH_METHOD(Array, pop) {
  ASSERT_NULL_AND_TYPE(Array, "Array.pop");
	return obj->get<Array>()->pop();
}

/*
 * @description Does nothing.
 * @argument None
 * @returns The array itself.
 */
EH_METHOD(Array, toArray) {
  ASSERT_NULL_AND_TYPE(Array, "Array.toArray");
  return obj;
}

/*
 * @description Converts the array into a tuple of all values in the array.
 * @argument None
 * @returns Tuple of all values in the array.
 */
EH_METHOD(Array, toTuple) {
	ASSERT_NULL_AND_TYPE(Array, "Array.toTuple");
	Array::t *arr = obj->get<Array>();
	unsigned int length = static_cast<unsigned int>(arr->size());
	ehretval_a values(length);
	for(int i = 0; i < (int) length; i++) {
		values[i] = arr->v[i];
	}
	return Tuple::make(length, values, ehi->get_parent());
}

/*
 * @description Returns an iterator over the array. The elements are
 * iterated over in an unspecified order.
 * @argument None
 * @returns Array.Iterator object.
 */
EH_METHOD(Array, getIterator) {
	ASSERT_NULL_AND_TYPE(Array, "Array.getIterator");
	ehval_p class_member = obj->get_type_object(ehi->get_parent())->get_property("Iterator", obj, ehi);
	return ehi->call_method(class_member, "operator()", obj, obj);
}

/*
 * @description Compare two arrays. Currently implemented by concurrently
 * iterating over each array and comparing each key, then each value.
 * @argument Array to compare to.
 * @returns Integer, as specified for Object.compare.
 */
EH_METHOD(Array, compare) {
	ASSERT_OBJ_TYPE(Array, "Array.compare");
	args->assert_type<Array>("Array.compare", ehi);
	Array::t *lhs = obj->get<Array>();
	Array::t *rhs = args->get<Array>();
	return Integer::make(lhs->compare(rhs, obj, ehi));
}

EH_INITIALIZER(Array_Iterator) {
	REGISTER_CONSTRUCTOR(Array_Iterator);
	REGISTER_METHOD(Array_Iterator, hasNext);
	REGISTER_METHOD(Array_Iterator, next);
	REGISTER_METHOD(Array_Iterator, peek);
}

ehval_p Array_Iterator::make(ehval_p array, EHInterpreter *parent) {
	return parent->allocate<Array_Iterator>(new t(array));
}

Array_Iterator::t::t(ehval_p _array) : array(_array) {
	Array::t *arr = array->get<Array>();
	this->it_begin = arr->v.begin();
	this->it_end = arr->v.end();
}
bool Array_Iterator::t::has_next() const {
	return this->it_begin != this->it_end;
}
ehval_p Array_Iterator::t::next(EHI *ehi) {
	assert(this->has_next());
	ehval_p out = *(this->it_begin);
	this->it_begin++;
	return out;
}
ehval_p Array_Iterator::t::peek(EHI *ehi) const {
	assert(this->has_next());
	return *(this->it_begin);
}

EH_METHOD(Array_Iterator, operator_colon) {
	args->assert_type<Array>("Array.Iterator.operator()", ehi);
	return Array_Iterator::make(args, ehi->get_parent());
}
EH_METHOD(Array_Iterator, hasNext) {
	args->assert_type<Null>("Array.Iterator.hasNext", ehi);
	ASSERT_RESOURCE(Array_Iterator, "Array.Iterator.hasNext");
	return Bool::make(data->has_next());
}
EH_METHOD(Array_Iterator, next) {
	args->assert_type<Null>("Array.Iterator.next", ehi);
	ASSERT_RESOURCE(Array_Iterator, "Array.Iterator.next");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next(ehi);
}
EH_METHOD(Array_Iterator, peek) {
	args->assert_type<Null>("Array.Iterator.peek", ehi);
	ASSERT_RESOURCE(Array_Iterator, "Array.Iterator.peek");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->peek(ehi);
}

