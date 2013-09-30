/*
 * ArgumentError
 * Thrown when a function gets an invalid argument. Used by many library
 * methods; may also be used by user code.
 */
#include <sstream>

#include "ArgumentError.hpp"
#include "Exception.hpp"

EH_NORETURN void throw_ArgumentError(const char *message, const char *method, ehval_p value, EHI *ehi) {
	ehval_p args[3];
	args[0] = String::make(strdup(message));
	args[1] = String::make(strdup(method));
	args[2] = value;
	ehval_p the_tuple = Tuple::make(3, args, ehi->get_parent());
	throw_error("ArgumentError", the_tuple, ehi);
}

EH_INITIALIZER(ArgumentError) {
	REGISTER_STATIC_METHOD_RENAME(ArgumentError, operator_colon, "operator()");
	INHERIT_LIBRARY(Exception);
}

/*
 * @description Initializer. Puts its arguments in the object properties
 * <tt>message</tt>, <tt>method</tt>, and <tt>value</tt>.
 * @argument Tuple of 3: message, method throwing the exception, and value
 * that triggered the exception.
 * @returns N/A
 */
EH_METHOD(ArgumentError, operator_colon) {
	ASSERT_NARGS(3, "ArgumentError()");
	ehval_p message = args->get<Tuple>()->get(0);
	message->assert_type<String>("ArgumentError()", ehi);
	obj->set_property("message", message, ehi->global(), ehi);

	ehval_p method = args->get<Tuple>()->get(1);
	method->assert_type<String>("ArgumentError()", ehi);
	obj->set_property("method", method, ehi->global(), ehi);

	ehval_p value = args->get<Tuple>()->get(2);
	obj->set_property("value", value, ehi->global(), ehi);

	std::ostringstream exception_msg;
	exception_msg << message->get<String>() << " (method ";
	exception_msg << method->get<String>() << "): ";
	exception_msg << ehi->toString(value, ehi->global())->get<String>();
	return Exception::make(strdup(exception_msg.str().c_str()));
}
