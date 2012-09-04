#include "UnknownCommandError.h"

void throw_UnknownCommandError(const char *msg, EHI *ehi) {
	throw_error("UnknownCommandError", ehretval_t::make_string(strdup(msg)), ehi);
}

START_EHLC(UnknownCommandError)
EHLC_ENTRY(UnknownCommandError, initialize)
EHLC_ENTRY(UnknownCommandError, toString)
END_EHLC()

EH_METHOD(UnknownCommandError, initialize) {
	ASSERT_TYPE(args, string_e, "UnknownCommandError.initialize");
	ehi->set_property(obj, "command", args, ehi->global_object);
	std::string msg = std::string("Unknown command: ") + args->get_stringval();
	return ehretval_t::make_resource(new Exception(strdup(msg.c_str())));
}
EH_METHOD(UnknownCommandError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "UnknownCommandError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}
