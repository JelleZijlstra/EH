#include <sstream>

#include "SyntaxError.h"

void throw_SyntaxError(const char *message, int line, EHI *ehi) {
	ehretval_p args[2];
	args[0] = ehretval_t::make_string(strdup(message));
	args[1] = ehretval_t::make_int(line);
	throw_error("SyntaxError", ehi->make_tuple(new ehtuple_t(2, args)), ehi);
}

START_EHLC(SyntaxError)
EHLC_ENTRY(SyntaxError, initialize)
EHLC_ENTRY(SyntaxError, toString)
END_EHLC()

EH_METHOD(SyntaxError, initialize) {
	ASSERT_NARGS(2, "SyntaxError.initialize");
	ehretval_p message = args->get_tupleval()->get(0);
	ASSERT_TYPE(message, string_e, "SyntaxError.initialize");
	ehi->set_property(obj, "errorMessage", message, ehi->global_object);

	ehretval_p line = args->get_tupleval()->get(1);
	ASSERT_TYPE(line, int_e, "SyntaxError.initialize");
	ehi->set_property(obj, "line", line, ehi->global_object);

	std::ostringstream exception_msg;
	exception_msg << message->get_stringval() << " at line ";
	exception_msg << line->get_intval();
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));	
}
EH_METHOD(SyntaxError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "SyntaxError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));	
}
