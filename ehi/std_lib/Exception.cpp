#include "Exception.h"

EH_INITIALIZER(Exception) {
	REGISTER_METHOD(Exception, initialize);
	REGISTER_METHOD(Exception, toString);
}

EH_METHOD(Exception, initialize) {
	ASSERT_TYPE(args, string_e, "Exception.initialize");
	Exception *e = new Exception(strdup(args->get_stringval()));
	return ehretval_t::make_resource(e);
}
EH_METHOD(Exception, toString) {
	ASSERT_NULL_AND_TYPE(resource_e, "Exception.toString");
	Exception *exc = static_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(exc->msg));
}
