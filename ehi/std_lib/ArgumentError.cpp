/*
 * ArgumentError
 * Thrown when a function gets an invalid argument. Used by many library
 * methods; may also be used by user code.
 */
#include <sstream>

#include "ArgumentError.h"

void throw_ArgumentError(const char *message, const char *method, ehretval_p value, EHI *ehi) {
	ehretval_p args[3];
	args[0] = ehretval_t::make_string(strdup(message));
	args[1] = ehretval_t::make_string(strdup(method));
	args[2] = value;
	ehretval_p the_tuple = ehi->get_parent()->make_tuple(new ehtuple_t(3, args));
	throw_error("ArgumentError", the_tuple, ehi);
}

EH_INITIALIZER(ArgumentError) {
	REGISTER_METHOD(ArgumentError, initialize);
	INHERIT_LIBRARY(Exception);
}

/*
 * @description Initializer. Puts its arguments in the object properties
 * <tt>message</tt>, <tt>method</tt>, and <tt>value</tt>.
 * @argument Tuple of 3: message, method throwing the exception, and value
 * that triggered the exception.
 * @returns N/A
 */
EH_METHOD(ArgumentError, initialize) {
	ASSERT_NARGS(3, "ArgumentError.initialize");
	ehretval_p message = args->get_tupleval()->get(0);
	ASSERT_TYPE(message, string_e, "ArgumentError.initialize");
	ehi->set_property(obj, "message", message, ehi->global());

	ehretval_p method = args->get_tupleval()->get(1);
	ASSERT_TYPE(method, string_e, "ArgumentError.initialize");
	ehi->set_property(obj, "method", method, ehi->global());

	ehretval_p value = args->get_tupleval()->get(2);
	ehi->set_property(obj, "value", value, ehi->global());

	std::ostringstream exception_msg;
	exception_msg << message->get_stringval() << " (method ";
	exception_msg << method->get_stringval() << "): ";
	exception_msg << ehi->to_string(value, ehi->global())->get_stringval();
	Exception *e = new Exception(strdup(exception_msg.str().c_str()));
	return ehretval_t::make_resource(obj->get_full_type(), e);
}
