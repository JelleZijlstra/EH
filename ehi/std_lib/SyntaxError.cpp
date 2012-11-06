#include <sstream>

#include "SyntaxError.h"

void throw_SyntaxError(const char *message, int line, EHI *ehi) {
	ehretval_p args[2];
	args[0] = ehretval_t::make_string(strdup(message));
	args[1] = ehretval_t::make_int(line);
	ehretval_p arg = ehi->get_parent()->make_tuple(new ehtuple_t(2, args));
	throw_error("SyntaxError", arg, ehi);
}

EH_INITIALIZER(SyntaxError) {
	REGISTER_METHOD(SyntaxError, initialize);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(SyntaxError, initialize) {
	ASSERT_NARGS(2, "SyntaxError.initialize");
	ehretval_p message = args->get_tupleval()->get(0);
	ASSERT_TYPE(message, string_e, "SyntaxError.initialize");
	ehi->set_property(obj, "errorMessage", message, ehi->global());

	ehretval_p line = args->get_tupleval()->get(1);
	ASSERT_TYPE(line, int_e, "SyntaxError.initialize");
	ehi->set_property(obj, "line", line, ehi->global());

	std::ostringstream exception_msg;
	exception_msg << message->get_stringval() << " at line ";
	exception_msg << line->get_intval();
	Exception *e = new Exception(strdup(exception_msg.str().c_str()));
	return ehretval_t::make_resource(obj->get_full_type(), e);
}
