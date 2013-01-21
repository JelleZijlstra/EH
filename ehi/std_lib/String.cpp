#include <regex>

#include "String.hpp"

#include "ArgumentError.hpp"
#include "EmptyIterator.hpp"
#include "Range.hpp"

EH_INITIALIZER(String) {
	REGISTER_METHOD(String, initialize);
	REGISTER_METHOD_RENAME(String, operator_plus, "operator+");
	REGISTER_METHOD_RENAME(String, operator_arrow, "operator->");
	REGISTER_METHOD(String, compare);
	REGISTER_METHOD(String, length);
	REGISTER_METHOD(String, toString);
	REGISTER_METHOD(String, toInteger);
	REGISTER_METHOD(String, toFloat);
	REGISTER_METHOD(String, toBool);
	REGISTER_METHOD(String, toRange);
	REGISTER_METHOD(String, charAtPosition);
	REGISTER_METHOD(String, getIterator);
	REGISTER_METHOD(String, trim);
	REGISTER_METHOD(String, doesMatch);
	REGISTER_METHOD(String, replace);
	REGISTER_CLASS(String, Iterator);
	REGISTER_CLASS(String, Builder);
}

EH_METHOD(String, initialize) {
	return ehi->toString(args, obj);
}
EH_METHOD(String, operator_plus) {
	ASSERT_OBJ_TYPE(String, "String.operator+");
	ehval_p operand = ehi->toString(args, obj);
	operand->assert_type<String>("String.operator+", ehi);
	size_t len1 = strlen(obj->get<String>());
	size_t len2 = strlen(operand->get<String>());
	char *out = new char[len1 + len2 + 1];
	strcpy(out, obj->get<String>());
	strcpy(out + len1, operand->get<String>());
	return String::make(out);
}
EH_METHOD(String, operator_arrow) {
	ASSERT_OBJ_TYPE(String, "String.operator->");
	args->assert_type<Integer>("String.operator->", ehi);
	int index = args->get<Integer>();
	size_t len = strlen(obj->get<String>());
	// allow negative index
	if(index < 0) {
		index += len;
	}
	if(index < 0 || ((unsigned) index) >= len) {
		throw_ArgumentError_out_of_range("String.operator->", args, ehi);
	}
	char *out = new char[2]();
	out[0] = obj->get<String>()[index];
	out[1] = '\0';
	return String::make(out);
}
EH_METHOD(String, compare) {
	ASSERT_OBJ_TYPE(String, "String.compare");
	args->assert_type<String>("String.compare", ehi);
	const char *lhs = obj->get<String>();
	const char *rhs = args->get<String>();
	int comparison = strcmp(lhs, rhs);
	// strcmp may return any negative or positive integer; standardize on -1 and 1 for EH
	if(comparison < 0) {
		comparison = -1;
	} else if(comparison > 0) {
		comparison = 1;
	}
	return Integer::make(comparison);
}
EH_METHOD(String, length) {
	ASSERT_NULL_AND_TYPE(String, "String.length");
	return Integer::make(static_cast<Integer::type>(strlen(obj->get<String>())));
}
EH_METHOD(String, toString) {
	ASSERT_NULL_AND_TYPE(String, "String.toString");
	return obj;
}
EH_METHOD(String, toInteger) {
	ASSERT_NULL_AND_TYPE(String, "String.toInteger");
	char *endptr;
	ehval_p ret = Integer::make(static_cast<Integer::type>(strtol(obj->get<String>(), &endptr, 0)));
	// If in == endptr, strtol read no digits and there was no conversion.
	if(obj->get<String>() == endptr) {
		throw_ArgumentError("Cannot convert String to Integer", "String.toInteger", obj, ehi);
	}
	return ret;
}
EH_METHOD(String, toFloat) {
	ASSERT_NULL_AND_TYPE(String, "String.toFloat");
	char *endptr;
	ehval_p ret = Float::make(strtof(obj->get<String>(), &endptr));
	// If in == endptr, strtof read no digits and there was no conversion.
	if(obj->get<String>() == endptr) {
		throw_ArgumentError("Cannot convert String to Float", "String.toFloat", obj, ehi);
	}
	return ret;
}
EH_METHOD(String, toBool) {
	ASSERT_NULL_AND_TYPE(String, "String.toBool");
	return Bool::make(obj->get<String>()[0] != '\0');
}
EH_METHOD(String, toRange) {
	ASSERT_NULL_AND_TYPE(String, "String.toRange");
	// attempt to find two integers in the string
	int min, max;
	const char *in = obj->get<String>();
	char *ptr;
	// get lower part of range
	for(int i = 0; ; i++) {
		if(in[i] == '\0') {
			throw_ArgumentError("Cannot convert String to Range", "String.toRange", obj, ehi);
		}
		if(isdigit(in[i])) {
			min = static_cast<Integer::type>(strtol(&in[i], &ptr, 0));
			break;
		}
	}
	// get upper bound
	for(int i = 0; ; i++) {
		if(ptr[i] == '\0') {
			throw_ArgumentError("Cannot convert String to Range", "String.toRange", obj, ehi);
		}
		if(isdigit(ptr[i])) {
			max = static_cast<Integer::type>(strtol(&ptr[i], nullptr, 0));
			break;
		}
	}
	return Range::make(Integer::make(min), Integer::make(max), ehi->get_parent());
}
EH_METHOD(String, charAtPosition) {
	ASSERT_OBJ_TYPE(String, "String.charAtPosition");
	args->assert_type<Integer>("String.charAtPosition", ehi);
	int index = args->get<Integer>();
	const char *string = obj->get<String>();
	if(index < 0 || ((unsigned) index) >= strlen(string)) {
		throw_ArgumentError_out_of_range("String.charAtPosition", args, ehi);
	}
	return Integer::make(string[index]);
}
EH_METHOD(String, getIterator) {
	ASSERT_NULL_AND_TYPE(String, "String.getIterator");
	ehval_p class_member = obj->get_property("Iterator", obj, ehi);
	return ehi->call_method(class_member, "new", obj, obj);
}

/*
 * @description Trim leading and trailing whitespace from a string.
 * @argument None
 * @returns New string
 */
EH_METHOD(String, trim) {
	ASSERT_RESOURCE(String, "String.trim");
	const size_t starting_len = strlen(data);
	const char *start_ptr = data;
	const char *end_ptr = start_ptr + starting_len;
	// left-trim
	while(start_ptr < end_ptr && isspace(start_ptr[0])) {
		start_ptr++;
	}
	// right trim
	while(end_ptr > start_ptr && isspace(end_ptr[-1])) {
		end_ptr--;
	}
	const size_t new_len = static_cast<size_t>(end_ptr - start_ptr);
	if(starting_len == new_len) {
		// just return the existing string
		return obj;
	} else {
		char *new_str = new char[new_len + 1];
		strncpy(new_str, start_ptr, new_len);
		new_str[new_len] = '\0';
		return String::make(new_str);
	}
}


/*
 * Regex operations
 */
typedef std::match_results<const char*> match;

const std::regex get_regex(ehval_p args, const char *method, EHI *ehi) {
	ASSERT_TYPE(args, String, method);
	const char *str = args->get<String>();
	try {
		return std::regex{str};
	} catch(const std::regex_error &) {
		throw_ArgumentError("Invalid regular expression", method, args, ehi);
	}
}

/*
 * @description Checks whether the string matches the given regular expression.
 * @argument Regular expression
 * @returns Bool
 */
EH_METHOD(String, doesMatch) {
	ASSERT_RESOURCE(String, "String.doesMatch");
	auto rgx = get_regex(args, "String.doesMatch", ehi);
	std::string str{data};
	return Bool::make(std::regex_search(str.begin(), str.end(), rgx));
}

/*
 * @description Replaces a regex pattern in a string.
 * @argument Tuple of size 2: regular expression and replacement pattern
 * @returns Modified string
 */
EH_METHOD(String, replace) {
	ASSERT_RESOURCE(String, "String.replace");
	ASSERT_NARGS(2, "String.replace");
	ehval_p pattern = args->get<Tuple>()->get(0);
	auto rgx = get_regex(pattern, "String.replace", ehi);
	ehval_p replacement = args->get<Tuple>()->get(1);
	ASSERT_TYPE(replacement, String, "String.replace");
	std::string str{data};
	std::string repl{replacement->get<String>()};
	std::string out = std::regex_replace(str, rgx, repl);
	return String::make(strdup(out.c_str()));
}

EH_INITIALIZER(String_Iterator) {
	REGISTER_METHOD(String_Iterator, initialize);
	REGISTER_METHOD(String_Iterator, hasNext);
	REGISTER_METHOD(String_Iterator, next);
	REGISTER_METHOD(String_Iterator, peek);
}

ehval_p String_Iterator::make(ehval_p string, EHInterpreter *parent) {
	return parent->allocate<String_Iterator>(new t(string));
}

bool String_Iterator::t::has_next() const {
	const char *content = this->string->get<String>();
	return this->position < strlen(content);
}
char String_Iterator::t::next() {
	assert(this->has_next());
	const char *content = this->string->get<String>();
	return content[this->position++];
}
char String_Iterator::t::peek() const {
	assert(this->has_next());
	const char *content = this->string->get<String>();
	return content[this->position];
}

EH_METHOD(String_Iterator, initialize) {
	args->assert_type<String>("String.Iterator.initialize", ehi);
	return String_Iterator::make(args, ehi->get_parent());
}
EH_METHOD(String_Iterator, hasNext) {
	args->assert_type<Null>("String.Iterator.hasNext", ehi);
	ASSERT_RESOURCE(String_Iterator, "String.Iterator.hasNext");
	return Bool::make(data->has_next());
}
EH_METHOD(String_Iterator, next) {
	args->assert_type<Null>("String.Iterator.next", ehi);
	ASSERT_RESOURCE(String_Iterator, "String.Iterator.next");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	char *out = new char[2]();
	out[0] = data->next();
	out[1] = '\0';
	return String::make(out);
}
EH_METHOD(String_Iterator, peek) {
	args->assert_type<Null>("String.Iterator.next", ehi);
	ASSERT_RESOURCE(String_Iterator, "String.Iterator.peek");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	char *out = new char[2]();
	out[0] = data->peek();
	out[1] = '\0';
	return String::make(out);
}


EH_INITIALIZER(String_Builder) {
	REGISTER_METHOD(String_Builder, new);
	REGISTER_METHOD_RENAME(String_Builder, operator_leftshift, "operator<<");
	REGISTER_METHOD(String_Builder, toString);
}

/*
 * @description Creates a new String.Builder object
 * @argument None
 * @returns String.Builder
 */
EH_METHOD(String_Builder, new) {
	return String_Builder::make();
}

/*
 * @description Adds an item to the String.Builder
 * @argument Any object
 * @returns The String.Builder object
 */
EH_METHOD(String_Builder, operator_leftshift) {
	ASSERT_RESOURCE(String_Builder, "String.Builder.operator<<");
	data->add_string(ehi->toString(args, obj)->get<String>());
	return obj;
}

/*
 * @description Converts the buffer to a string
 * @argument None
 * @returns String
 */
EH_METHOD(String_Builder, toString) {
	ASSERT_RESOURCE(String_Builder, "String.Builder.operator<<");
	return String::make(strdup(data->get_string().c_str()));
}
