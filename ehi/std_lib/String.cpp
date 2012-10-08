#include "String.h"

EH_INITIALIZER(String) {
	REGISTER_METHOD(String, initialize);
	REGISTER_METHOD_RENAME(String, operator_plus, "operator+");
	REGISTER_METHOD_RENAME(String, operator_arrow, "operator->");
	REGISTER_METHOD_RENAME(String, operator_arrow_equals, "operator->=");
	REGISTER_METHOD(String, compare);
	REGISTER_METHOD(String, length);
	REGISTER_METHOD(String, toString);
	REGISTER_METHOD(String, toInt);
	REGISTER_METHOD(String, toFloat);
	REGISTER_METHOD(String, toBool);
	REGISTER_METHOD(String, toRange);
	REGISTER_METHOD(String, charAtPosition);
	REGISTER_METHOD(String, getIterator);
	REGISTER_CLASS(String, Iterator);
}

EH_METHOD(String, initialize) {
	return ehi->to_string(args, obj);
}
EH_METHOD(String, operator_plus) {
	ASSERT_OBJ_TYPE(string_e, "String.operator+");
	ehretval_p operand = ehi->to_string(args, obj);
	ASSERT_TYPE(operand, string_e, "String.operator+");
	size_t len1 = strlen(obj->get_stringval());
	size_t len2 = strlen(operand->get_stringval());
	char *out = new char[len1 + len2 + 1];
	strcpy(out, obj->get_stringval());
	strcpy(out + len1, operand->get_stringval());
	return ehretval_t::make_string(out); 
}
EH_METHOD(String, operator_arrow) {
	ASSERT_OBJ_TYPE(string_e, "String.operator->");
	ASSERT_TYPE(args, int_e, "String.operator->");
	int index = args->get_intval();
	size_t len = strlen(obj->get_stringval());
	// allow negative index
	if(index < 0) {
		index += len;
	}
	if(index < 0 || ((unsigned) index) >= len) {
		throw_ArgumentError_out_of_range("String.operator->", args, ehi);
	}
	char *out = new char[2]();
	out[0] = obj->get_stringval()[index];
	out[1] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(String, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, string_e, "String.operator->=");
	ehretval_p operand1 = args->get_tupleval()->get(0);
	ASSERT_TYPE(operand1, int_e, "String.operator->=");
	int index = operand1->get_intval();
	size_t len = strlen(obj->get_stringval());
	if(index < 0) {
		index += len;
	}
	if(index < 0 || ((unsigned) index) >= len) {
		throw_ArgumentError_out_of_range("String.operator->=", operand1, ehi);
	}
	ehretval_p operand2 = ehi->to_string(args->get_tupleval()->get(1), obj);
	if(strlen(operand2->get_stringval()) == 0) {
		throw_ArgumentError("Argument cannot be a zero-length string", "String.operator->=", args->get_tupleval()->get(1), ehi);
	}
	obj->get_stringval()[index] = operand2->get_stringval()[0];
	return operand2;
}
EH_METHOD(String, compare) {
	ASSERT_OBJ_TYPE(string_e, "String.compare");
	ASSERT_TYPE(args, string_e, "String.compare");
	const char *lhs = obj->get_stringval();
	const char *rhs = args->get_stringval();
	int comparison = strcmp(lhs, rhs);
	// strcmp may return any negative or positive integer; standardize on -1 and 1 for EH
	if(comparison < 0) {
		comparison = -1;
	} else if(comparison > 0) {
		comparison = 1;
	}
	return ehretval_t::make_int(comparison);
}
EH_METHOD(String, length) {
	ASSERT_NULL_AND_TYPE(string_e, "String.length");
	return ehretval_t::make_int(strlen(obj->get_stringval()));
}
EH_METHOD(String, toString) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toString");
	return obj;
}
EH_METHOD(String, toInt) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toInt");
	char *endptr;
	ehretval_p ret = ehretval_t::make_int(strtol(obj->get_stringval(), &endptr, 0));
	// If in == endptr, strtol read no digits and there was no conversion.
	if(obj->get_stringval() == endptr) {
		throw_ArgumentError("Cannot convert String to Integer", "String.toInt", obj, ehi);
	}
	return ret;
}
EH_METHOD(String, toFloat) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toFloat");
	char *endptr;
	ehretval_p ret = ehretval_t::make_float(strtof(obj->get_stringval(), &endptr));
	// If in == endptr, strtof read no digits and there was no conversion.
	if(obj->get_stringval() == endptr) {
		throw_ArgumentError("Cannot convert String to Float", "String.toFloat", obj, ehi);
	}
	return ret;
}
EH_METHOD(String, toBool) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toBool");
	return ehretval_t::make_bool(obj->get_stringval()[0] != '\0');
}
EH_METHOD(String, toRange) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toRange");
	// attempt to find two integers in the string
	int min, max;
	char *in = obj->get_stringval();
	char *ptr;
	// get lower part of range
	for(int i = 0; ; i++) {
		if(in[i] == '\0') {
			throw_ArgumentError("Cannot convert String to Range", "String.toRange", obj, ehi);
		}
		if(isdigit(in[i])) {
			min = strtol(&in[i], &ptr, 0);
			break;
		}
	}
	// get upper bound
	for(int i = 0; ; i++) {
		if(ptr[i] == '\0') {
			throw_ArgumentError("Cannot convert String to Range", "String.toRange", obj, ehi);
		}
		if(isdigit(ptr[i])) {
			max = strtol(&ptr[i], NULL, 0);
			break;
		}
	}
	ehrange_t *range = new ehrange_t(ehretval_t::make_int(min), ehretval_t::make_int(max));
	return ehi->get_parent()->make_range(range);
}
EH_METHOD(String, charAtPosition) {
	ASSERT_OBJ_TYPE(string_e, "String.charAtPosition");
	ASSERT_TYPE(args, int_e, "String.charAtPosition");
	int index = args->get_intval();
	const char *string = obj->get_stringval();
	if(index < 0 || ((unsigned) index) >= strlen(string)) {
		throw_ArgumentError_out_of_range("String.charAtPosition", args, ehi);
	}
	return ehretval_t::make_int(string[index]);
}
EH_METHOD(String, getIterator) {
	ASSERT_NULL_AND_TYPE(string_e, "String.getIterator");
	ehretval_p class_member = ehi->get_property(obj, "Iterator", obj);
	return ehi->call_method(class_member, "new", obj, obj);
}

EH_INITIALIZER(String_Iterator) {
	REGISTER_METHOD(String_Iterator, initialize);
	REGISTER_METHOD(String_Iterator, hasNext);
	REGISTER_METHOD(String_Iterator, next);
	REGISTER_METHOD(String_Iterator, peek);
}

bool String_Iterator::has_next() const {
	const char *string = this->string->get_stringval();
	return this->position < strlen(string);
}
char String_Iterator::next() {
	assert(this->has_next());
	const char *string = this->string->get_stringval();
	return string[this->position++];	
}
char String_Iterator::peek() const {
	assert(this->has_next());
	const char *string = this->string->get_stringval();
	return string[this->position];
}

EH_METHOD(String_Iterator, initialize) {
	ASSERT_TYPE(args, string_e, "String.Iterator.initialize");
	String_Iterator *data = new String_Iterator(args);
	return ehretval_t::make_resource(data);
}
EH_METHOD(String_Iterator, hasNext) {
	ASSERT_TYPE(args, null_e, "String.Iterator.hasNext");
	String_Iterator *data = (String_Iterator *)obj->get_resourceval();
	return ehretval_t::make_bool(data->has_next());
}
EH_METHOD(String_Iterator, next) {
	ASSERT_TYPE(args, null_e, "String.Iterator.next");
	String_Iterator *data = (String_Iterator *)obj->get_resourceval();
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	char *out = new char[2]();
	out[0] = data->next();
	out[1] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(String_Iterator, peek) {
	ASSERT_TYPE(args, null_e, "String.Iterator.next");
	String_Iterator *data = (String_Iterator *)obj->get_resourceval();
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	char *out = new char[2]();
	out[0] = data->peek();
	out[1] = '\0';
	return ehretval_t::make_string(out);
}
