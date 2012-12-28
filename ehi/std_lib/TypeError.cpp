#include "TypeError.hpp"

void throw_TypeError(const char *msg, int type, EHI *ehi) {
	ehretval_p args[2];
	args[0] = ehretval_t::make_string(strdup(msg));
	args[1] = ehretval_t::make_int(type);
	ehretval_p arg = ehi->get_parent()->make_tuple(new ehtuple_t(2, args));
	throw_error("TypeError", arg, ehi);
}

EH_INITIALIZER(TypeError) {
	REGISTER_METHOD(TypeError, initialize);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(TypeError, initialize) {
	ASSERT_TYPE(args, tuple_e, "TypeError.initialize");
	ehretval_p msg = args->get_tupleval()->get(0);
	ASSERT_TYPE(msg, string_e, "TypeError.initialize");
	ehretval_p id = args->get_tupleval()->get(1);
	ASSERT_TYPE(id, int_e, "TypeError.initialize");
	ehi->set_property(obj, "message", msg, ehi->global());
	std::string type_str = ehi->get_parent()->repo.get_name(id->get_intval());
	ehi->set_property(obj, "type", id, ehi->global());
	std::string exception_msg = std::string(msg->get_stringval()) + ": " + type_str;
	return ehretval_t::make_resource(obj->get_full_type(), new Exception(strdup(exception_msg.c_str())));
}
