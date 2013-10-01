#include "UnknownCommandError.hpp"

#include "Exception.hpp"

void throw_UnknownCommandError(const char *msg, EHI *ehi) {
	throw_error("UnknownCommandError", String::make(strdup(msg)), ehi);
}

EH_INITIALIZER(UnknownCommandError) {
	REGISTER_METHOD(UnknownCommandError, initialize);
	INHERIT_PURE_CLASS(Exception);
}

EH_METHOD(UnknownCommandError, initialize) {
	args->assert_type<String>("UnknownCommandError()", ehi);
	obj->set_property("command", args, ehi->get_parent()->global_object, ehi);
	std::string msg = std::string("Unknown command: ") + args->get<String>();
	obj->set_property("message", String::make(strdup(msg.c_str())), ehi->global(), ehi);
	return nullptr;
}
