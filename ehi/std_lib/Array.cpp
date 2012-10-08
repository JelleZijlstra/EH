#include "Array.h"

EH_INITIALIZER(Array) {
	REGISTER_METHOD(Array, initialize);
	REGISTER_METHOD(Array, has);
	REGISTER_METHOD(Array, length);
	REGISTER_METHOD_RENAME(Array, operator_arrow, "operator->");
	REGISTER_METHOD_RENAME(Array, operator_arrow_equals, "operator->=");
	REGISTER_METHOD(Array, toArray);
	REGISTER_METHOD(Array, toTuple);
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

EH_METHOD(Array, initialize) {
	return ehi->to_array(args, obj);
}
EH_METHOD(Array, length) {
	ASSERT_NULL_AND_TYPE(array_e, "Array.length");
	return ehretval_t::make_int(obj->get_arrayval()->size());
}
EH_METHOD(Array, has) {
	ASSERT_OBJ_TYPE(array_e, "Array.has");
	return ehretval_t::make_bool(obj->get_arrayval()->has(args));
}
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
  return ehi->get_parent()->make_tuple(new ehtuple_t(length, values));
}
EH_METHOD(Array, getIterator) {
	ASSERT_NULL_AND_TYPE(array_e, "Array.getIterator");
	ehretval_p class_member = ehi->get_property(obj, "Iterator", obj);
	return ehi->call_method(class_member, "new", obj, obj);
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
	return ehretval_t::make_resource(data);
}
EH_METHOD(Array_Iterator, hasNext) {
	ASSERT_TYPE(args, null_e, "Array.Iterator.hasNext");
	Array_Iterator *data = (Array_Iterator *)obj->get_resourceval();
	return ehretval_t::make_bool(data->has_next());
}
EH_METHOD(Array_Iterator, next) {
	ASSERT_TYPE(args, null_e, "Array.Iterator.next");
	Array_Iterator *data = (Array_Iterator *)obj->get_resourceval();
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next(ehi);
}
EH_METHOD(Array_Iterator, peek) {
	ASSERT_TYPE(args, null_e, "Array.Iterator.peek");
	Array_Iterator *data = (Array_Iterator *)obj->get_resourceval();
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->peek(ehi);
}

