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
	paras->int_indices[0]->print();
	printf("\n");
	return NULL;
}

EH_LIBCMD(put) {
	paras->int_indices[0]->print();
	return NULL;
}
