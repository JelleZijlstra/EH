#include "UnknownCommandError.hpp"

#include "Exception.hpp"

void throw_UnknownCommandError(const char *msg, EHI *ehi) {
	throw_error("UnknownCommandError", String::make(strdup(msg)), ehi);
}

EH_INITIALIZER(UnknownCommandError) {
	REGISTER_CONSTRUCTOR(UnknownCommandError);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(UnknownCommandError, operator_colon) {
	args->assert_type<String>("UnknownCommandError()", ehi);
	obj->set_property("command", args, ehi->get_parent()->global_object, ehi);
	std::string msg = std::string("Unknown command: ") + args->get<String>();
	return Exception::make(strdup(msg.c_str()));
}
