/*
 * eh_error.c
 * Jelle Zijlstra, December 2011
 *
 * Code file for the EH error handling system.
 */
#include "eh.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include "std_lib/Exception.h"

/*
 * Exception classes
 */
void throw_error(const char *class_name, ehretval_p args, EHI *ehi) {
	ehretval_p class_member = ehi->get_property(ehi->global_object, class_name, ehcontext_t(ehi->global_object, ehi->global_object));
	ehretval_p e = ehi->call_method(class_member, "new", args, ehcontext_t(ehi->global_object, ehi->global_object));
	throw eh_exception(e);
}
