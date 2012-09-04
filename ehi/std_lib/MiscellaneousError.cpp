#include "MiscellaneousError.h"

void throw_MiscellaneousError(const char *message, EHI *ehi) {
	throw_error("MiscellaneousError", ehretval_t::make_string(strdup(message)), ehi);
}

START_EHLC(MiscellaneousError)
EHLC_ENTRY(MiscellaneousError, initialize)
EHLC_ENTRY(MiscellaneousError, toString)
END_EHLC()

EH_METHOD(MiscellaneousError, initialize) {
	ASSERT_TYPE(args, string_e, "MiscellaneousError.initialize");
	ehi->set_property(obj, "message", args, ehi->global_object);
	return ehretval_t::make_resource(new Exception(strdup(args->get_stringval())));
}
EH_METHOD(MiscellaneousError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "MiscellaneousError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}
