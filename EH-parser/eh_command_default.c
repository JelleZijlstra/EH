/*
 * eh_command_default.c
 * Jelle Zijlstra, December 2011
 *
 * Default implementation of EH commands, used when PHP commands are not
 * available.
 */
#include "eh_error.h"

void execute_command(char *name, ehvar_t **array) {
	eh_error("Use of commands outside of EH-PHP context", eerror_e);
	return;
}
