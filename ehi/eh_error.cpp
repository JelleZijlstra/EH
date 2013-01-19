/*
 * eh_error.c
 * Jelle Zijlstra, December 2011
 *
 * Code file for the EH error handling system.
 */

#include "eh.hpp"

/*
 * Exception classes
 */
void throw_error(const char *class_name, ehval_p args, EHI *ehi) [[noreturn]] {
	ehval_p global_object = ehi->get_parent()->global_object;
	ehval_p class_member = global_object->get_property(class_name, global_object, ehi);
	ehval_p e = ehi->call_method(class_member, "new", args, global_object);
	throw eh_exception(e);
}
