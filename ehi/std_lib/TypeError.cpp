#include "TypeError.hpp"

#include "Exception.hpp"

void throw_TypeError(const char *msg, ehval_p obj, EHI *ehi) {
	ehval_p args[2];
	args[0] = String::make(strdup(msg));
	args[1] = obj;
	throw_error("TypeError", Tuple::make(2, args, ehi->get_parent()), ehi);
}

EH_INITIALIZER(TypeError) {
	REGISTER_METHOD(TypeError, initialize);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(TypeError, initialize) {
	args->assert_type<Tuple>("TypeError.initialize", ehi);

	ehval_p msg = args->get<Tuple>()->get(0);
	msg->assert_type<String>("TypeError.initialize", ehi);
	obj->set_property("message", msg, ehi->global(), ehi);

	ehval_p id = args->get<Tuple>()->get(1);
	std::string type_str = ehi->get_parent()->repo.get_name(id);
	ehval_p type_obj = ehi->get_parent()->repo.get_object(id);
	obj->set_property("type", type_obj, ehi->global(), ehi);
	std::string exception_msg = std::string(msg->get<String>()) + ": " + type_str;
	return Exception::make(strdup(exception_msg.c_str()));
}
