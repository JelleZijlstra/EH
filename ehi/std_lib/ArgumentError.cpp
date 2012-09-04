#include <sstream>

#include "ArgumentError.h"

void throw_ArgumentError(const char *message, const char *method, ehretval_p value, EHI *ehi) {
	ehretval_p args[3];
	args[0] = ehretval_t::make_string(strdup(message));
	args[1] = ehretval_t::make_string(strdup(method));
	args[2] = value;
	ehretval_p the_tuple = ehi->make_tuple(new ehtuple_t(3, args));
	throw_error("ArgumentError", the_tuple, ehi);
}

START_EHLC(ArgumentError)
EHLC_ENTRY(ArgumentError, initialize)
EHLC_ENTRY(ArgumentError, toString)
END_EHLC()

EH_METHOD(ArgumentError, initialize) {
	ASSERT_NARGS(3, "ArgumentError.initialize");
	ehretval_p message = args->get_tupleval()->get(0);
	ASSERT_TYPE(message, string_e, "ArgumentError.initialize");
	ehi->set_property(obj, "message", message, ehi->global_object);

	ehretval_p method = args->get_tupleval()->get(1);
	ASSERT_TYPE(method, string_e, "ArgumentError.initialize");
	ehi->set_property(obj, "method", method, ehi->global_object);

	ehretval_p value = args->get_tupleval()->get(2);
	ehi->set_property(obj, "value", value, ehi->global_object);

	std::ostringstream exception_msg;
	exception_msg << message->get_stringval() << " (method " << method->get_stringval() << "): ";
	exception_msg << ehi->to_string(value, ehi->global_object)->get_stringval();
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));	
}
EH_METHOD(ArgumentError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "ArgumentError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}
