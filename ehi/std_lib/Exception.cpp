/*
 * Exception
 * Base class of all exceptions thrown by the engine. Although EH allows
 * throwing any value, it is good practice to only throw instances of
 * Exception or classes deriving from it.
 */
#include "Exception.hpp"

[[noreturn]] void throw_error(const char *class_name, ehval_p args, EHI *ehi) {
	ehval_p global_object = ehi->get_parent()->global_object;
	ehval_p class_member = global_object->get_property(class_name, global_object, ehi);
	ehval_p e = ehi->call_method(class_member, "new", args, global_object);
	throw eh_exception(e);
}

EH_INITIALIZER(Exception) {
	REGISTER_METHOD(Exception, initialize);
	REGISTER_METHOD(Exception, toString);
}

/*
 * @description Initializer.
 * @argument Exception message
 * @returns N/A
 */
EH_METHOD(Exception, initialize) {
	args->assert_type<String>("Exception.initialize", ehi);
	return Exception::make(strdup(args->get<String>()));
}

/*
 * @description Converts the exception to string, returning the message passed
 * in to initialize.
 * @arguments None
 * @returns String
 */
EH_METHOD(Exception, toString) {
	args->assert_type<Null>("Exception.toString", ehi);
	ASSERT_RESOURCE(Exception, "Exception.toString");
	return String::make(strdup(data));
}
