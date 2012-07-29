#ifndef _EH_ERROR_H
#define _EH_ERROR_H

#include "eh_libclasses.h"
/*
 * EH error system
 */
typedef enum {
	efatal_e,
	eparsing_e,
	eerror_e, // runtime non-fatal error
	enotice_e
} errlevel_e;

void eh_error(const char *message, errlevel_e level);
void eh_error_type(const char *context, type_enum type, errlevel_e level);
void eh_error_looplevels(const char *context, int levels);
void eh_error_unknown(const char *kind, const char *name, errlevel_e level);
void eh_error_redefine(const char *kind, const char *name, errlevel_e level);
void eh_error_int(const char *message, int opcode, errlevel_e level);
void eh_error_argcount(int expected, int received);
void eh_error_line(int line, const char *msg);
void eh_error_types(const char *context, type_enum type1, type_enum type2, errlevel_e level);
void eh_error_argcount_lib(const char *name, int expected, int received);
void eh_error_invalid_argument(const char *function, int n);

/*
 * Exception types.
 */
class quit_exception : public std::exception {
};
class unknown_value_exception : public std::exception {
};

/*
 * Exceptions throwns by the interpreter.
 */
// throw a generic error
void throw_error(const char *class_name, ehretval_p args, EHI *ehi);

// UnknownCommandError
void throw_UnknownCommandError(const char *msg, EHI *ehi);

EH_METHOD(UnknownCommandError, initialize);
EH_METHOD(UnknownCommandError, toString);

EXTERN_EHLC(UnknownCommandError)

// TypeError
void throw_TypeError(const char *msg, int type, EHI *ehi);

EH_METHOD(TypeError, initialize);
EH_METHOD(TypeError, toString);

EXTERN_EHLC(TypeError)

#endif /* _EH_ERROR_H */
