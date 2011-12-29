/*
 * eh_error.h
 * Jelle Zijlstra, December 2011
 *
 * Header file for the EH error handling system.
 */
#include "eh.h"

typedef enum {
	efatal_e,
	eparsing_e,
	eerror_e, // runtime non-fatal error
	enotice_e,
} errlevel_e;

void eh_error(char *message, errlevel_e level);
void eh_error_type(char *context, type_enum type, errlevel_e level);
void eh_error_looplevels(char *context, int levels);
void eh_error_unknown(char *kind, char *name, errlevel_e level);
void eh_error_redefine(char *kind, char *name, errlevel_e level);
void eh_error_int(char *message, int opcode, errlevel_e level);
void eh_error_argcount(int expected, int received);
void eh_error_line(int line, char *msg);
void eh_error_types(int context, type_enum type1, type_enum type2, errlevel_e level);
void eh_error_argcount_lib(char *name, int expected, int received);