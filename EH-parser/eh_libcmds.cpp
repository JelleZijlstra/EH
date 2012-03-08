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
	ehretval_t index;
	index.intval = 0;
	index.type = int_e;
	
	ehretval_t *arg = array_get(paras, &index);
	print_retval(arg);
	printf("\n");

	return NULL;
}

EH_LIBCMD(put) {
	ehretval_t index;
	index.intval = 0;
	index.type = int_e;
	
	ehretval_t *arg = array_get(paras, &index);
	print_retval(arg);

	return NULL;
}
