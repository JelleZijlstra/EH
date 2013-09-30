#include <sstream>

#include "VisibilityError.hpp"

#include "Exception.hpp"
#include "ArgumentError.hpp"

void throw_VisibilityError(ehval_p object, const char *name, EHI *ehi) {
	ehval_p args[2];
	args[0] = object;
	args[1] = String::make(strdup(name));
	throw_error("VisibilityError", Tuple::make(2, args, ehi->get_parent()), ehi);
}

EH_INITIALIZER(VisibilityError) {
	REGISTER_CONSTRUCTOR(VisibilityError);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(VisibilityError, operator_colon) {
	ASSERT_NARGS(2, "VisibilityError()");
	ehval_p object = args->get<Tuple>()->get(0);
	ehval_p name = args->get<Tuple>()->get(1);
	name->assert_type<String>("VisibilityError()", ehi);
	obj->set_property("object", object, ehi->global(), ehi);
	obj->set_property("name", name, ehi->global(), ehi);
	std::ostringstream exception_msg;
	const unsigned int type_id = obj->get_type_id(ehi->get_parent());
	exception_msg << "Cannot access private member " << name->get<String>() << " in object of type " << ehi->get_parent()->repo.get_name(type_id);
	exception_msg << ": " << ehi->toString(object, ehi->global())->get<String>();
	return Exception::make(strdup(exception_msg.str().c_str()));
}
