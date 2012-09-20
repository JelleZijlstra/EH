#include <sstream>

#include "ConstError.h"

void throw_ConstError(ehretval_p object, const char *name, EHI *ehi) {
	ehretval_p args[2] = {object, ehretval_t::make_string(strdup(name))};
	throw_error("ConstError", ehi->get_parent()->make_tuple(new ehtuple_t(2, args)), ehi);
}

EH_INITIALIZER(ConstError) {
	REGISTER_METHOD(ConstError, initialize);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(ConstError, initialize) {
	ASSERT_NARGS(2, "ConstError.initialize");
	ehretval_p object = args->get_tupleval()->get(0);
	ehretval_p name = args->get_tupleval()->get(1);
	ASSERT_TYPE(name, string_e, "ConstError.initialize");
	ehi->set_property(obj, "object", object, ehi->global());
	ehi->set_property(obj, "name", name, ehi->global());
	std::ostringstream exception_msg;
	exception_msg << "Cannot set constant member " << name->get_stringval() << " in object of type " << object->type_string(ehi);
	exception_msg << ": " << ehi->to_string(object, ehi->global())->get_stringval();
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));
}
