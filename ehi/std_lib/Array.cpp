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
#include "Array.h"

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

ehretval_p &eharray_t::operator[](ehretval_p index) {
	switch(index->type()) {
		case int_e:
			return int_indices[index->get_intval()];
		case string_e:
			return string_indices[index->get_stringval()];
		default:
			// callers should make sure type is right
			assert(false);
			throw "impossible";
	}
}
void eharray_t::insert_retval(ehretval_p index, ehretval_p value) {
	// Inserts a member into an array.
	switch(index->type()) {
		case int_e:
			this->int_indices[index->get_intval()] = value;
			break;
		case string_e:
			this->string_indices[index->get_stringval()] = value;
			break;
		default:
			// callers should make sure type is right
			assert(false);
	}
}

int eharray_t::compare(eharray_t *rhs, ehcontext_t context, EHI *ehi) {
	const int lhs_size = this->size();
	const int rhs_size = rhs->size();
	if(lhs_size != rhs_size) {
		return intcmp(lhs_size, rhs_size);
	}
	// compare integer keys
	eharray_t::int_iterator lhs_int_it = this->int_indices.begin();
	eharray_t::int_iterator rhs_int_it = rhs->int_indices.begin();
	eharray_t::int_iterator lhs_int_end = this->int_indices.end();
	eharray_t::int_iterator rhs_int_end = rhs->int_indices.end();
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
	eharray_t::string_iterator lhs_string_it = this->string_indices.begin();
	eharray_t::string_iterator rhs_string_it = rhs->string_indices.begin();
	eharray_t::string_iterator lhs_string_end = this->string_indices.end();
	eharray_t::string_iterator rhs_string_end = rhs->string_indices.end();
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
	return ehi->to_array(args, obj);
}

/*
 * @description Length of an array.
 * @argument None.
 * @returns Number of items in the array.
 */
EH_METHOD(Array, length) {
	ASSERT_NULL_AND_TYPE(array_e, "Array.length");
	return ehretval_t::make_int(obj->get_arrayval()->size());
}

/*
 * @description Checks whether a given key exists in the array.
 * @argument String or integer key.
 * @returns True if key exists, false if not.
 */
EH_METHOD(Array, has) {
	ASSERT_OBJ_TYPE(array_e, "Array.has");
	return ehretval_t::make_bool(obj->get_arrayval()->has(args));
}

/*
 * @description Access the element with the given key
 * @argument String or integer key.
 * @returns Value associated with the key, or null if key does not exist.
 */
EH_METHOD(Array, operator_arrow) {
	ASSERT_OBJ_TYPE(array_e, "Array.operator->");
	if(args->type() != int_e && args->type() != string_e) {
		throw_TypeError("Invalid type for argument to Array.operator-> (expected String or Integer)", args->type(), ehi);
	}
	eharray_t *arr = obj->get_arrayval();
	if(arr->has(args)) {
		return (*arr)[args];
	} else {
		return NULL;
	}
}

/*
 * @description Set the element with the given key
 * @argument Tuple of size 2: key and value to set to
 * @returns Value set
 */
EH_METHOD(Array, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, array_e, "Array.operator->=");
	ehretval_p index = args->get_tupleval()->get(0);
	if(index->type() != int_e && index->type() != string_e) {
		throw_TypeError("Invalid type for argument to Array.operator->= (expected String or Integer)", index->type(), ehi);
	}
	ehretval_p rvalue = args->get_tupleval()->get(1);
	(*obj->get_arrayval())[index] = rvalue;
	return rvalue;
}

/*
 * @description Does nothing.
 * @argument None
 * @returns The array itself.
 */
EH_METHOD(Array, toArray) {
  ASSERT_NULL_AND_TYPE(array_e, "Array.toArray");
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
	return ehi->get_parent()->make_tuple(new ehtuple_t(length, values));
}

/*
 * @description Returns an iterator over the array. The elements are
 * iterated over in an unspecified order.
 * @argument None
 * @returns Array.Iterator object.
 */
EH_METHOD(Array, getIterator) {
	ASSERT_NULL_AND_TYPE(array_e, "Array.getIterator");
	ehretval_p class_member = ehi->get_property(obj, "Iterator", obj);
	return ehi->call_method(class_member, "new", obj, obj);
}

/*
 * @description Compare two arrays. Currently implemented by concurrently
 * iterating over each array and comparing each key, then each value.
 * @argument Array to compare to.
 * @returns Integer, as specified for Object.compare.
 */
EH_METHOD(Array, compare) {
	ASSERT_OBJ_TYPE(array_e, "Array.compare");
	ASSERT_TYPE(args, array_e, "Array.compare");
	args = ehretval_t::self_or_data(args);
	eharray_t *lhs = obj->get_arrayval();
	eharray_t *rhs = args->get_arrayval();
	return ehretval_t::make_int(lhs->compare(rhs, obj, ehi));
}

EH_INITIALIZER(Array_Iterator) {
	REGISTER_METHOD(Array_Iterator, initialize);
	REGISTER_METHOD(Array_Iterator, hasNext);
	REGISTER_METHOD(Array_Iterator, next);
}

Array_Iterator::Array_Iterator(ehretval_p array) {
	this->current_type = int_e;
	eharray_t *arr = array->get_arrayval();
	this->int_begin = arr->int_indices.begin();
	this->int_end = arr->int_indices.end();
	if(this->int_begin == this->int_end) {
		this->current_type = string_e;
	}
	this->string_begin = arr->string_indices.begin();
	this->string_end = arr->string_indices.end();
}
bool Array_Iterator::has_next() const {
	return !((this->current_type == string_e || (this->int_begin == this->int_end)) && (this->string_begin == this->string_end));
}
ehretval_p Array_Iterator::next(EHI *ehi) {
	assert(this->has_next());
	ehretval_p tuple[2];
	if(this->current_type == int_e) {
		tuple[0] = ehretval_t::make_int(this->int_begin->first);
		tuple[1] = this->int_begin->second;
		this->int_begin++;
		if(this->int_begin == this->int_end) {
			this->current_type = string_e;
		}
	} else {
		const char *key = this->string_begin->first.c_str();
		tuple[0] = ehretval_t::make_string(strdup(key));
		tuple[1] = this->string_begin->second;
		this->string_begin++;
	}
	return ehi->get_parent()->make_tuple(new ehtuple_t(2, tuple));
}
ehretval_p Array_Iterator::peek(EHI *ehi) const {
	assert(this->has_next());
	ehretval_p tuple[2];
	if(this->current_type == int_e) {
		tuple[0] = ehretval_t::make_int(this->int_begin->first);
		tuple[1] = this->int_begin->second;
	} else {
		const char *key = this->string_begin->first.c_str();
		tuple[0] = ehretval_t::make_string(strdup(key));
		tuple[1] = this->string_begin->second;
	}
	return ehi->get_parent()->make_tuple(new ehtuple_t(2, tuple));
}

EH_METHOD(Array_Iterator, initialize) {
	ASSERT_TYPE(args, array_e, "Array.Iterator.initialize");
	Array_Iterator *data = new Array_Iterator(args);
	return ehretval_t::make_resource(obj->get_full_type(), data);
}
EH_METHOD(Array_Iterator, hasNext) {
	ASSERT_TYPE(args, null_e, "Array.Iterator.hasNext");
	ASSERT_RESOURCE(Array_Iterator, "Array.Iterator.hasNext");
	return ehretval_t::make_bool(data->has_next());
}
EH_METHOD(Array_Iterator, next) {
	ASSERT_TYPE(args, null_e, "Array.Iterator.next");
	ASSERT_RESOURCE(Array_Iterator, "Array.Iterator.next");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next(ehi);
}
EH_METHOD(Array_Iterator, peek) {
	ASSERT_TYPE(args, null_e, "Array.Iterator.peek");
	ASSERT_RESOURCE(Array_Iterator, "Array.Iterator.peek");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->peek(ehi);
}

