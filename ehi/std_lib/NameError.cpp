#include <sstream>

#include "NameError.hpp"

#include "ArgumentError.hpp"
#include "Exception.hpp"

void throw_NameError(ehval_p object, const char *name, EHI *ehi) {
	ehval_p args[2];
	args[0] = object;
	args[1] = String::make(strdup(name));
	throw_error("NameError", Tuple::make(2, args, ehi->get_parent()), ehi);
}

EH_INITIALIZER(NameError) {
	REGISTER_METHOD(NameError, initialize);
	INHERIT_PURE_CLASS(Exception);
}

EH_METHOD(NameError, initialize) {
	ASSERT_NARGS(2, "NameError()");
	ehval_p object = args->get<Tuple>()->get(0);
	ehval_p name = args->get<Tuple>()->get(1);
	name->assert_type<String>("NameError()", ehi);
	obj->set_property("object", object, ehi->global(), ehi);
	obj->set_property("name", name, ehi->global(), ehi);
	std::ostringstream exception_msg;
	const unsigned int type_id = object->get_type_id(ehi->get_parent());
	exception_msg << "Unknown member " << name->get<String>() << " in object of type " << ehi->get_parent()->repo.get_name(type_id);
	exception_msg << ": " << ehi->toString(object, ehi->global())->get<String>();
	obj->set_property("message", String::make(strdup(exception_msg.str().c_str())), ehi->global(), ehi);
	return nullptr;
}
