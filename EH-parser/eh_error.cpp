/*
 * eh_error.c
 * Jelle Zijlstra, December 2011
 *
 * Code file for the EH error handling system.
 */
#include "eh.h"
#include "eh_error.h"
#include <stdio.h>
#include <stdlib.h>

void eh_error(const char *message, errlevel_e level) {
	if(message)
		fputs(message, stderr);
	switch(level) {
		case efatal_e:
			fprintf(stderr, ": EH fatal error\n");
			throw quit_exception();
		case eparsing_e:
			fprintf(stderr, ": EH parsing error\n");
			// TODO: figure out whether we're interactive
			break;
		case eerror_e:
			fprintf(stderr, ": EH runtime error\n");
			break;
		case enotice_e:
			fprintf(stderr, ": EH notice\n");
			break;
	}
}

void eh_error_type(const char *context, type_enum type, errlevel_e level) {
	fprintf(stderr, "Unsupported type %s for %s", get_typestring(type), context);
	eh_error(NULL, level);
}
void eh_error_types(const char *context, type_enum type1, type_enum type2, errlevel_e level) {
	fprintf(stderr, "Unsupported types %s and %s for operator %s", get_typestring(type1), get_typestring(type2), context);
	eh_error(NULL, level);
}
void eh_error_looplevels(const char *context, int levels) {
	fprintf(stderr, "%s for %d levels", context, levels);
	eh_error(NULL, eerror_e);
}
void eh_error_unknown(const char *kind, const char *name, errlevel_e level) {
	fprintf(stderr, "No such %s: %s", kind, name);
	eh_error(NULL, level);
}
void eh_error_redefine(const char *kind, const char *name, errlevel_e level) {
	fprintf(stderr, "Attempt to redefine %s: %s", kind, name);
	eh_error(NULL, level);
}
void eh_error_int(const char *message, int opcode, errlevel_e level) {
	fprintf(stderr, "%s: %d", message, opcode);
	eh_error(NULL, level);
}
void eh_error_argcount(int expected, int received) {
	fprintf(stderr, "Incorrect argument count for function: expected %d, got %d", expected, received);
	eh_error(NULL, eerror_e);
}
void eh_error_argcount_lib(const char *name, int expected, int received) {
	fprintf(stderr, "Incorrect argument count for function %s: expected %d, got %d", name, expected, received);
	eh_error(NULL, eerror_e);
}
void eh_error_line(int line, const char *msg) {
	fprintf(stderr, "In line %d: ", line);
	eh_error(msg, eparsing_e);
}
void eh_error_invalid_argument(const char *function, int n) {
  fptintf(stderr, "Invalid value for argument %d to %s", n, function);
  eh_error(msg, enotice_e);
}
