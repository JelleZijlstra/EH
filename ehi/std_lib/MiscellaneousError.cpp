#include "Exception.hpp"
#include "MiscellaneousError.hpp"
#include "Object.hpp"

void throw_MiscellaneousError(const char *message, EHI *ehi) {
	throw_error("MiscellaneousError", String::make(strdup(message)), ehi);
}

EH_INITIALIZER(MiscellaneousError) {
	REGISTER_METHOD(MiscellaneousError, initialize);
	INHERIT_PURE_CLASS(Exception);
}

EH_METHOD(MiscellaneousError, initialize) {
	args->assert_type<String>("MiscellaneousError()", ehi);
	obj->set_property("message", args, ehi->global(), ehi);
	return nullptr;
}
