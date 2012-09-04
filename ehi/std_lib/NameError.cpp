#include <sstream>

#include "NameError.h"

void throw_NameError(ehretval_p object, const char *name, EHI *ehi) {
	ehretval_p args[2];
	args[0] = object;
	args[1] = ehretval_t::make_string(strdup(name));
	throw_error("NameError", ehi->make_tuple(new ehtuple_t(2, args)), ehi);
}

START_EHLC(NameError)
EHLC_ENTRY(NameError, initialize)
EHLC_ENTRY(NameError, toString)
END_EHLC()

EH_METHOD(NameError, initialize) {
	ASSERT_NARGS(2, "NameError.initialize");
	ehretval_p object = args->get_tupleval()->get(0);
	ehretval_p name = args->get_tupleval()->get(1);
	ASSERT_TYPE(name, string_e, "NameError.initialize");
	ehi->set_property(obj, "object", object, ehi->global_object);
	ehi->set_property(obj, "name", name, ehi->global_object);
	std::ostringstream exception_msg;
	exception_msg << "Unknown member " << name->get_stringval() << " in object of type " << object->type_string(ehi);
	exception_msg << ": " << ehi->to_string(object, ehi->global_object)->get_stringval();
	return ehretval_t::make_resource(new Exception(strdup(exception_msg.str().c_str())));
}
EH_METHOD(NameError, toString) {
	ASSERT_OBJ_TYPE(resource_e, "NameError.toString");
	Exception *e = reinterpret_cast<Exception *>(obj->get_resourceval());
	return ehretval_t::make_string(strdup(e->msg));
}
