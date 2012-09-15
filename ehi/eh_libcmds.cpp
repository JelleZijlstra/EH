/*
 * Code for library commands.
 */
#include "eh.h"
#include "eh_libcmds.h"
#include "eh_libclasses.h"
#include "std_lib/GlobalObject.h"
#include "std_lib/Array.h"

EH_LIBCMD(quit) {
	throw quit_exception();
	return NULL;
}

EH_LIBCMD(echo) {
	ASSERT_TYPE(paras, array_e, "echo");
	return ehlm_GlobalObject_echo(obj, paras->get_arrayval()->int_indices[0], ehi);
}

EH_LIBCMD(put) {
	ASSERT_TYPE(paras, array_e, "put");
	return ehlm_GlobalObject_put(obj, paras->get_arrayval()->int_indices[0], ehi);
}
