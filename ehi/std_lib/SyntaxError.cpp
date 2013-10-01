#include <sstream>

#include "SyntaxError.hpp"

#include "ArgumentError.hpp"
#include "Exception.hpp"

EH_NORETURN void throw_SyntaxError(const char *message, int line, EHI *ehi) {
	ehval_p args[2];
	args[0] = String::make(strdup(message));
	args[1] = Integer::make(line);
	throw_error("SyntaxError", Tuple::make(2, args, ehi->get_parent()), ehi);
}

EH_INITIALIZER(SyntaxError) {
	REGISTER_METHOD(SyntaxError, initialize);
	INHERIT_PURE_CLASS(Exception);
}

EH_METHOD(SyntaxError, initialize) {
	ASSERT_NARGS(2, "SyntaxError()");
	ehval_p message = args->get<Tuple>()->get(0);
	message->assert_type<String>("SyntaxError()", ehi);
	obj->set_property("errorMessage", message, ehi->global(), ehi);

	ehval_p line = args->get<Tuple>()->get(1);
	line->assert_type<Integer>("SyntaxError()", ehi);
	obj->set_property("line", line, ehi->global(), ehi);

	std::ostringstream exception_msg;
	exception_msg << message->get<String>() << " at line ";
	exception_msg << line->get<Integer>();
	obj->set_property("message", String::make(strdup(exception_msg.str().c_str())), ehi->global(), ehi);
	return nullptr;
}
