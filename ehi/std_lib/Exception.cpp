/*
 * Exception
 * Base class of all exceptions thrown by the engine. Although EH allows
 * throwing any value, it is good practice to only throw instances of
 * Exception or classes deriving from it.
 */
#include "Exception.hpp"
#include "Class.hpp"

EH_NORETURN void throw_error(const char *class_name, ehval_p args, EHI *ehi) {
	ehval_p global_object = ehi->get_parent()->global_object;
	ehval_p class_member = global_object->get_property(class_name, global_object, ehi);
	ehval_p e = ehi->call_method(class_member, "operator()", args, global_object);
	throw eh_exception(e);
}

EH_INITIALIZER(Exception) {
	REGISTER_METHOD(Exception, initialize);
	REGISTER_METHOD(Exception, toString);
}

/*
 * @description Initializer.
 * @argument Exception message
 * @returns N/A
 */
EH_METHOD(Exception, initialize) {
	args->assert_type<String>("Exception()", ehi);
	ehmember_p member(attributes_t::make_const(), args);
	obj->set_member("message", member, obj, ehi);
	return nullptr;
}

/*
 * @description Converts the exception to string, returning the message passed
 * in to initialize.
 * @arguments None
 * @returns String
 */
EH_METHOD(Exception, toString) {
	args->assert_type<Null>("Exception.toString", ehi);
	// get exception type
	const unsigned int type_id = obj->get_type_id(ehi->get_parent());
	const std::string type_name = ehi->get_parent()->repo.get_name(type_id);
	const char *msg = ehi->toString(obj->get_property_no_binding("message", obj, ehi)->value, obj)->get<String>();
	return String::make(strdup(("<" + type_name + ": " + msg + ">").c_str()));
}

EH_NORETURN void throw_CompileError(const char *message, EHI *ehi) {
	ehval_p args = String::make(strdup(message));
	throw_error("CompileError", args, ehi);
}

EH_INITIALIZER(CompileError) {}

