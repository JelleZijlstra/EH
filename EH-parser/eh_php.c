/*
 * eh_php.c
 * Jelle Zijlstra, December 2011
 *
 * Implementation of EH as a PHP extension.
 */
#include "eh_error.h"

extern FILE *yyin;
void yyparse(void);

// Replace this with a PHP method call
void execute_command(char *name, ehvar_t **array) {
	eh_error("Use of commands outside of EH-PHP context", eerror_e);
	return;
}
