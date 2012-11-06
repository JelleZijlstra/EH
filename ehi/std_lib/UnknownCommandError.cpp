#include "UnknownCommandError.h"

void throw_UnknownCommandError(const char *msg, EHI *ehi) {
	throw_error("UnknownCommandError", ehretval_t::make_string(strdup(msg)), ehi);
}

EH_INITIALIZER(UnknownCommandError) {
	REGISTER_METHOD(UnknownCommandError, initialize);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(UnknownCommandError, initialize) {
	ASSERT_TYPE(args, string_e, "UnknownCommandError.initialize");
	ehi->set_property(obj, "command", args, ehi->get_parent()->global_object);
	std::string msg = std::string("Unknown command: ") + args->get_stringval();
	return ehretval_t::make_resource(obj->get_full_type(), new Exception(strdup(msg.c_str())));
}
