#include <sstream>

#include "LoopError.h"

void throw_LoopError(const char *msg, int level, EHI *ehi) {
	ehretval_p args[2];
	args[0] = ehretval_t::make_string(strdup(msg));
	args[1] = ehretval_t::make_int(level);
	throw_error("LoopError", ehi->get_parent()->make_tuple(new ehtuple_t(2, args)), ehi);	
}

EH_INITIALIZER(LoopError) {
	REGISTER_METHOD(LoopError, initialize);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(LoopError, initialize) {
	ASSERT_TYPE(args, tuple_e, "LoopError.initialize");
	ehretval_p msg = args->get_tupleval()->get(0);
	ASSERT_TYPE(msg, string_e, "LoopError.initialize");
	ehretval_p level = args->get_tupleval()->get(1);
	ASSERT_TYPE(level, int_e, "LoopError.initialize");
	ehi->set_property(obj, "message", msg, ehi->global());
	ehi->set_property(obj, "level", level, ehi->global());
	std::ostringstream exception_msg;
	exception_msg << "Cannot " << msg->get_stringval() << " " << level->get_intval() << " levels";
	return ehretval_t::make_resource(obj->get_full_type(), new Exception(strdup(exception_msg.str().c_str())));
}
