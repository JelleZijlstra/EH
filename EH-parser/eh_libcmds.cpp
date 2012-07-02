/*
 * Code for library commands.
 */
#include "eh.h"
#include "eh_libcmds.h"

EH_LIBCMD(quit) {
	throw new std::exception;
	return NULL;
}

EH_LIBCMD(echo) {
	ehretval_t *arg = paras->int_indices[0];
	print_retval(arg);
	printf("\n");
	return NULL;
}

EH_LIBCMD(put) {
	ehretval_t *arg = paras->int_indices[0];
	print_retval(arg);
	return NULL;
}
