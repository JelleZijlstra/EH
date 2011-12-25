/*
 * eh_libfuncs.c
 * Jelle Zijlstra, December 2011
 *
 * Contains definitions of EH library functions
 */
#include "eh.h"

EHLIBFUNC(getinput) {
	fscanf(stdin, "%d", retval);
	return;
}