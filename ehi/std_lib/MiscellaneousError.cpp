#include "MiscellaneousError.hpp"

#include "Exception.hpp"

void throw_MiscellaneousError(const char *message, EHI *ehi) {
	throw_error("MiscellaneousError", String::make(strdup(message)), ehi);
}

EH_INITIALIZER(MiscellaneousError) {
	REGISTER_CONSTRUCTOR(MiscellaneousError);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(MiscellaneousError, operator_colon) {
	args->assert_type<String>("MiscellaneousError()", ehi);
	obj->set_property("message", args, ehi->global(), ehi);
	return Exception::make(strdup(args->get<String>()));
}
