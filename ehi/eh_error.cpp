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
void throw_error(const char *class_name, ehretval_p args, EHI *ehi) {
	ehretval_p global_object = ehi->get_parent()->global_object;
	ehretval_p class_member = ehi->get_property(global_object, class_name, global_object);
	ehretval_p e = ehi->call_method(class_member, "new", args, global_object);
	throw eh_exception(e);
}
