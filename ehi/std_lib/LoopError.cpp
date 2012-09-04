#include <sstream>

#include "LoopError.h"

void throw_LoopError(const char *msg, int level, EHI *ehi) {
	ehretval_p args[2];
	args[0] = ehretval_t::make_string(strdup(msg));
	args[1] = ehretval_t::make_int(level);
	throw_error("LoopError", ehi->make_tuple(new ehtuple_t(2, args)), ehi);	
}

START_EHLC(LoopError)
EHLC_ENTRY(LoopError, initialize)
EHLC_ENTRY(LoopError, toString)
END_EHLC()

EH_METHOD(LoopError, initialize) {
	ASSERT_TYPE(args, tuple_e, "LoopError.initialize");
	ehretval_p msg = args->get_tupleval()->get(0);
	ASSERT_TYPE(msg, string_e, "LoopError.initialize");
	ehretval_p level = args->get_tupleval()->get(1);
	ASSERT_TYPE(level, int_e, "LoopError.initialize");
	ehi->set_property(obj, "message", msg, ehi->global_object);
	ehi->set_property(obj, "level", level, ehi->global_object);
	std::ostringstream exception_msg;
	exception_msg << "Cannot " << msg->get_stringval() << " " << level->get_intval() << " levels";
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));
}
EH_METHOD(LoopError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "LoopError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}
