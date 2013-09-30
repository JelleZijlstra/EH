#include <sstream>

#include "LoopError.hpp"

#include "Exception.hpp"

void throw_LoopError(const char *msg, int level, EHI *ehi) {
	ehval_p args[2];
	args[0] = String::make(strdup(msg));
	args[1] = Integer::make(level);
	throw_error("LoopError", Tuple::make(2, args, ehi->get_parent()), ehi);
}

EH_INITIALIZER(LoopError) {
	REGISTER_CONSTRUCTOR(LoopError);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(LoopError, operator_colon) {
	args->assert_type<Tuple>("LoopError.initialize", ehi);
	ehval_p msg = args->get<Tuple>()->get(0);
	msg->assert_type<String>("LoopError.initialize", ehi);
	ehval_p level = args->get<Tuple>()->get(1);
	level->assert_type<Integer>("LoopError.initialize", ehi);
	obj->set_property("message", msg, ehi->global(), ehi);
	obj->set_property("level", level, ehi->global(), ehi);
	std::ostringstream exception_msg;
	exception_msg << "Cannot " << msg->get<String>() << " " << level->get<Integer>() << " levels";
	return Exception::make(strdup(exception_msg.str().c_str()));
}
