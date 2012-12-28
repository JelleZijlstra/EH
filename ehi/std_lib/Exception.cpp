#include "Exception.hpp"

EH_INITIALIZER(Exception) {
	REGISTER_METHOD(Exception, initialize);
	REGISTER_METHOD(Exception, toString);
}

EH_METHOD(Exception, initialize) {
	ASSERT_TYPE(args, string_e, "Exception.initialize");
	Exception *e = new Exception(strdup(args->get_stringval()));
	return ehretval_t::make_resource(obj->get_full_type(), e);
}
EH_METHOD(Exception, toString) {
	ASSERT_TYPE(args, null_e, "Exception.toString");
	ASSERT_RESOURCE(Exception, "Exception.toString");
	return ehretval_t::make_string(strdup(data->msg));
}
