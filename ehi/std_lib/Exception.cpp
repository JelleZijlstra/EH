/*
 * Exception
 * Base class of all exceptions thrown by the engine. Although EH allows
 * throwing any value, it is good practice to only throw instances of
 * Exception or classes deriving from it.
 */
#include "Exception.hpp"

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
