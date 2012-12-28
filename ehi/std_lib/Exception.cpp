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
	ASSERT_TYPE(args, string_e, "Exception.initialize");
	Exception *e = new Exception(strdup(args->get_stringval()));
	return ehretval_t::make_resource(obj->get_full_type(), e);
}

/*
 * @description Converts the exception to string, returning the message passed
 * in to initialize.
 * @arguments None
 * @returns String
 */
EH_METHOD(Exception, toString) {
	ASSERT_TYPE(args, null_e, "Exception.toString");
	ASSERT_RESOURCE(Exception, "Exception.toString");
	return ehretval_t::make_string(strdup(data->msg));
}
