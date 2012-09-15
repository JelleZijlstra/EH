#include "Array.h"

START_EHLC(Array)
	EHLC_ENTRY(Array, initialize)
	EHLC_ENTRY(Array, has)
	EHLC_ENTRY(Array, length)
	EHLC_ENTRY_RENAME(Array, operator_arrow, "operator->")
	EHLC_ENTRY_RENAME(Array, operator_arrow_equals, "operator->=")
	EHLC_ENTRY(Array, toArray)
	EHLC_ENTRY(Array, toTuple)
	EHLC_ENTRY(Array, getIterator)
	obj->register_member_class("Iterator", -1, ehinit_Array_Iterator, attributes_t::make(), ehi);
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
EH_METHOD(Array, getIterator) {
	ASSERT_NULL_AND_TYPE(array_e, "Array.getIterator");
	ehretval_p class_member = ehi->get_property(obj, "Iterator", obj);
	return ehi->call_method(class_member, "new", obj, obj);
}

START_EHLC(Array_Iterator)
EHLC_ENTRY(Array_Iterator, initialize)
EHLC_ENTRY(Array_Iterator, hasNext)
EHLC_ENTRY(Array_Iterator, next)
END_EHLC()

Array_Iterator::Array_Iterator(ehretval_p array) {
	this->current_type = int_e;
	eharray_t *arr = array->get_arrayval();
	this->int_begin = arr->int_indices.begin();
	this->int_end = arr->int_indices.end();
	this->string_begin = arr->string_indices.begin();
	this->string_end = arr->string_indices.end();
}
bool Array_Iterator::has_next() {
	return !((this->current_type == string_e || (this->int_begin == this->int_end)) && (this->string_begin == this->string_end));
}
ehretval_p Array_Iterator::next(EHI *ehi) {
	assert(this->has_next());
	if(this->current_type == int_e && this->int_begin == this->int_end) {
		this->current_type = string_e;
	}
	ehretval_p tuple[2];
	if(this->current_type == int_e) {
		tuple[0] = ehretval_t::make_int(this->int_begin->first);
		tuple[1] = this->int_begin->second;
		this->int_begin++;
	} else {
		const char *key = this->string_begin->first.c_str();
		tuple[0] = ehretval_t::make_string(strdup(key));
		tuple[1] = this->string_begin->second;
		this->string_begin++;	
	}
	return ehi->make_tuple(new ehtuple_t(2, tuple));
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


