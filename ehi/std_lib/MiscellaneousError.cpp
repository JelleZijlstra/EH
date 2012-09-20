#include "MiscellaneousError.h"

void throw_MiscellaneousError(const char *message, EHI *ehi) {
	throw_error("MiscellaneousError", ehretval_t::make_string(strdup(message)), ehi);
}

EH_INITIALIZER(MiscellaneousError) {
	REGISTER_METHOD(MiscellaneousError, initialize);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(MiscellaneousError, initialize) {
	ASSERT_TYPE(args, string_e, "MiscellaneousError.initialize");
	ehi->set_property(obj, "message", args, ehi->global());
	return ehretval_t::make_resource(new Exception(strdup(args->get_stringval())));
}
