#include "Exception.h"

START_EHLC(Exception)
EHLC_ENTRY(Exception, initialize)
EHLC_ENTRY(Exception, toString)
END_EHLC()

EH_METHOD(Exception, initialize) {
	ASSERT_TYPE(args, string_e, "Exception.initialize");
	return ehretval_t::make_resource(new Exception(strdup(args->get_stringval())));
}
EH_METHOD(Exception, toString) {
	ASSERT_NULL_AND_TYPE(resource_e, "Exception.toString");
	Exception *exc = (Exception *)obj->get_resourceval();
	return ehretval_t::make_string(strdup(exc->msg));
}
