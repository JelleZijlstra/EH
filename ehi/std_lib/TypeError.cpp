#include "TypeError.h"

void throw_TypeError(const char *msg, int type, EHI *ehi) {
	ehretval_p args[2];
	args[0] = ehretval_t::make_string(strdup(msg));
	args[1] = ehretval_t::make_int(type);
	throw_error("TypeError", ehi->make_tuple(new ehtuple_t(2, args)), ehi);
}

START_EHLC(TypeError)
EHLC_ENTRY(TypeError, initialize)
EHLC_ENTRY(TypeError, toString)
END_EHLC()

EH_METHOD(TypeError, initialize) {
	ASSERT_TYPE(args, tuple_e, "TypeError.initialize");
	ehretval_p msg = args->get_tupleval()->get(0);
	ASSERT_TYPE(msg, string_e, "TypeError.initialize");
	ehretval_p id = args->get_tupleval()->get(1);
	ASSERT_TYPE(id, int_e, "TypeError.initialize");
	ehi->set_property(obj, "message", msg, ehi->global_object);
	std::string type_str = ehi->repo.get_name(id->get_intval());
	ehi->set_property(obj, "type", id, ehi->global_object);
	std::string exception_msg = std::string(msg->get_stringval()) + ": " + type_str;
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.c_str())));
}
EH_METHOD(TypeError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "TypeError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}

