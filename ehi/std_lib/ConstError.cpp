/*
 * ConstError
 * Thrown when attempting to modify a constant property.
 */
#include <sstream>

#include "ConstError.hpp"

#include "ArgumentError.hpp"
#include "Exception.hpp"

void throw_ConstError(ehval_p object, const char *name, EHI *ehi) {
	ehval_p args[2] = {object, String::make(strdup(name))};
	throw_error("ConstError", Tuple::make(2, args, ehi->get_parent()), ehi);
}

EH_INITIALIZER(ConstError) {
	REGISTER_CONSTRUCTOR(ConstError);
	INHERIT_LIBRARY(Exception);
}

/*
 * @description Initializer. Puts its arguments in the object properties
 * <tt>object</tt> and <tt>name</tt>.
 * @argument Tuple of 2: object owning the property and name of the property
 * @returns N/A
 */
EH_METHOD(ConstError, operator_colon) {
	ASSERT_NARGS(2, "ConstError()");
	ehval_p object = args->get<Tuple>()->get(0);
	ehval_p name = args->get<Tuple>()->get(1);
	name->assert_type<String>("ConstError()", ehi);
	obj->set_property("object", object, ehi->global(), ehi);
	obj->set_property("name", name, ehi->global(), ehi);
	std::ostringstream exception_msg;
	const std::string type_name = ehi->get_parent()->repo.get_name(object->get_type_id(ehi->get_parent()));
	exception_msg << "Cannot set constant member " << name->get<String>() << " in object of type " << type_name;
	exception_msg << ": " << ehi->toString(object, ehi->global())->get<String>();
	return Exception::make(strdup(exception_msg.str().c_str()));
}
