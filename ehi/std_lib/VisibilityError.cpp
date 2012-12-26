#include <sstream>

#include "VisibilityError.h"

void throw_VisibilityError(ehretval_p object, const char *name, EHI *ehi) {
	ehretval_p args[2];
	args[0] = object;
	args[1] = ehretval_t::make_string(strdup(name));
	ehretval_p arg = ehi->get_parent()->make_tuple(new ehtuple_t(2, args));
	throw_error("VisibilityError", arg, ehi);
}

EH_INITIALIZER(VisibilityError) {
	REGISTER_METHOD(VisibilityError, initialize);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(VisibilityError, initialize) {
	ASSERT_NARGS(2, "VisibilityError.initialize");
	ehretval_p object = args->get_tupleval()->get(0);
	ehretval_p name = args->get_tupleval()->get(1);
	ASSERT_TYPE(name, string_e, "VisibilityError.initialize");
	ehi->set_property(obj, "object", object, ehi->global());
	ehi->set_property(obj, "name", name, ehi->global());
	std::ostringstream exception_msg;
	exception_msg << "Cannot access private member " << name->get_stringval() << " in object of type " << object->type_string(ehi);
	exception_msg << ": " << ehi->to_string(object, ehi->global())->get_stringval();
	return ehretval_t::make_resource(obj->get_full_type(), new Exception(strdup(exception_msg.str().c_str())));
}
