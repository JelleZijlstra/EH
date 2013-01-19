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

EH_INITIALIZER(Array) {
	REGISTER_METHOD(Array, initialize);
	REGISTER_METHOD(Array, has);
	REGISTER_METHOD(Array, length);
	REGISTER_METHOD_RENAME(Array, operator_arrow, "operator->");
	REGISTER_METHOD_RENAME(Array, operator_arrow_equals, "operator->=");
	REGISTER_METHOD(Array, toArray);
	REGISTER_METHOD(Array, toTuple);
	REGISTER_METHOD(Array, compare);
	REGISTER_METHOD(Array, getIterator);
	REGISTER_CLASS(Array, Iterator);
}

ehval_p Array::make(EHInterpreter *parent) {
	return parent->allocate<Array>(new t());
}

ehval_w &Array::t::operator[](ehval_p index) {
	if(index->is_a<Integer>()) {
		return int_indices[index->get<Integer>()];
	} else if(index->is_a<String>()) {
		return string_indices[index->get<String>()];
	} else {
		// callers should make sure type is right
		assert(false);
	}
}
void Array::t::insert_retval(ehval_p index, ehval_p val) {
	// Inserts a member into an array.
	(*this)[index] = val;
}

int Array::t::compare(Array::t *rhs, ehcontext_t context, EHI *ehi) {
	const size_t lhs_size = this->size();
	const size_t rhs_size = rhs->size();
	if(lhs_size != rhs_size) {
		return intcmp(lhs_size, rhs_size);
	}
	// compare integer keys
	Array::t::int_iterator lhs_int_it = this->int_indices.begin();
	Array::t::int_iterator rhs_int_it = rhs->int_indices.begin();
	Array::t::int_iterator lhs_int_end = this->int_indices.end();
	Array::t::int_iterator rhs_int_end = rhs->int_indices.end();
	while(true) {
		// check whether we've reached the end
		if(lhs_int_it == lhs_int_end) {
			if(rhs_int_it == rhs_int_end) {
				break;
			} else {
				return -1;
			}
		} else if(rhs_int_it == rhs_int_end) {
			return 1;
		}

		// compare current key
		if(lhs_int_it->first != rhs_int_it->first) {
			return intcmp(lhs_int_it->first, rhs_int_it->first);
		}

		// compare associated values
		int comparison = ehi->compare(lhs_int_it->second, rhs_int_it->second, context);
		if(comparison != 0) {
			return comparison;
		}

		// continue iteration
		lhs_int_it++;
		rhs_int_it++;
	}

	// compare string keys
	Array::t::string_iterator lhs_string_it = this->string_indices.begin();
	Array::t::string_iterator rhs_string_it = rhs->string_indices.begin();
	Array::t::string_iterator lhs_string_end = this->string_indices.end();
	Array::t::string_iterator rhs_string_end = rhs->string_indices.end();
	while(true) {
		// check whether we've reached the end
		if(lhs_string_it == lhs_string_end) {
			if(rhs_string_it == rhs_string_end) {
				break;
			} else {
				return -1;
			}
		} else if(rhs_string_it == rhs_string_end) {
			return 1;
		}

		// compare current key
		if(lhs_string_it->first != rhs_string_it->first) {
			return lhs_string_it->first.compare(rhs_string_it->first);
		}

		// compare associated values
		int comparison = ehi->compare(lhs_string_it->second, rhs_string_it->second, context);
		if(comparison != 0) {
			return comparison;
		}

		// continue iteration
		lhs_string_it++;
		rhs_string_it++;
	}
	return 0;
}

/*
 * @description Converts its argument to an array by calling its toArray method.
 * @argument Anything with a toArray method.
 * @returns Array.
 */
EH_METHOD(Array, initialize) {
	return ehi->call_method_typed<Array>(args, "toArray", nullptr, obj);
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
	return Bool::make(obj->get<Array>()->has(args));
}

/*
 * @description Access the element with the given key
 * @argument String or integer key.
 * @returns Value associated with the key, or null if key does not exist.
 */
EH_METHOD(Array, operator_arrow) {
	ASSERT_OBJ_TYPE(Array, "Array.operator->");
	if(!args->is_a<Integer>() && !args->is_a<String>()) {
		throw_TypeError("Invalid type for argument to Array.operator-> (expected String or Integer)", args, ehi);
	}
	Array::t *arr = obj->get<Array>();
	if(arr->has(args)) {
		return (*arr)[args];
	} else {
		return nullptr;
	}
}

/*
 * @description Set the element with the given key
 * @argument Tuple of size 2: key and value to set to
 * @returns Value set
 */
EH_METHOD(Array, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, Array, "Array.operator->=");
	ehval_p index = args->get<Tuple>()->get(0);
	if(!index->is_a<Integer>() && !index->is_a<String>()) {
		throw_TypeError("Invalid type for argument to Array.operator->= (expected String or Integer)", index, ehi);
	}
	ehval_p rvalue = args->get<Tuple>()->get(1);
	(*obj->get<Array>())[index] = rvalue;
	return rvalue;
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
 * The order in which the array elements appear in the output element is
 * unspecified.
 * @argument None
 * @returns Tuple of all values in the array.
 */
EH_METHOD(Array, toTuple) {
	ASSERT_NULL_AND_TYPE(Array, "Array.toTuple");
	Array::t *arr = obj->get<Array>();
	unsigned int length = static_cast<unsigned int>(arr->size());
	ehretval_a values(length);
	// We'll say that output order is unspecified
	unsigned int i = 0;
	for(auto &it : arr->int_indices) {
		values[i++] = it.second;
	}
	for(auto &it : arr->string_indices) {
		values[i++] = it.second;
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
	ehval_p class_member = obj->get_property("Iterator", obj, ehi);
	return ehi->call_method(class_member, "new", obj, obj);
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
	args = args->data();
	Array::t *lhs = obj->get<Array>();
	Array::t *rhs = args->get<Array>();
	return Integer::make(lhs->compare(rhs, obj, ehi));
}

EH_INITIALIZER(Array_Iterator) {
	REGISTER_METHOD(Array_Iterator, initialize);
	REGISTER_METHOD(Array_Iterator, hasNext);
	REGISTER_METHOD(Array_Iterator, next);
}

ehval_p Array_Iterator::make(ehval_p array, EHInterpreter *parent) {
	return parent->allocate<Array_Iterator>(new t(array));
}

Array_Iterator::t::t(ehval_p _array) : array(_array), in_ints(true) {
	Array::t *arr = array->get<Array>();
	this->int_begin = arr->int_indices.begin();
	this->int_end = arr->int_indices.end();
	if(this->int_begin == this->int_end) {
		this->in_ints = false;
	}
	this->string_begin = arr->string_indices.begin();
	this->string_end = arr->string_indices.end();
}
bool Array_Iterator::t::has_next() const {
	return !((this->in_ints == false || (this->int_begin == this->int_end)) && (this->string_begin == this->string_end));
}
ehval_p Array_Iterator::t::next(EHI *ehi) {
	assert(this->has_next());
	ehval_p tuple[2];
	if(this->in_ints) {
		tuple[0] = Integer::make(this->int_begin->first);
		tuple[1] = this->int_begin->second;
		this->int_begin++;
		if(this->int_begin == this->int_end) {
			this->in_ints = false;
		}
	} else {
		const char *key = this->string_begin->first.c_str();
		tuple[0] = String::make(strdup(key));
		tuple[1] = this->string_begin->second;
		this->string_begin++;
	}
	return Tuple::make(2, tuple, ehi->get_parent());
}
ehval_p Array_Iterator::t::peek(EHI *ehi) const {
	assert(this->has_next());
	ehval_p tuple[2];
	if(this->in_ints) {
		tuple[0] = Integer::make(this->int_begin->first);
		tuple[1] = this->int_begin->second;
	} else {
		const char *key = this->string_begin->first.c_str();
		tuple[0] = String::make(strdup(key));
		tuple[1] = this->string_begin->second;
	}
	return Tuple::make(2, tuple, ehi->get_parent());
}

EH_METHOD(Array_Iterator, initialize) {
	args->assert_type<Array>("Array.Iterator.initialize", ehi);
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

