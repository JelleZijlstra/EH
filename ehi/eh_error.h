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

// common variants
static inline void throw_TypeError_Array_key(int type, EHI *ehi) {
	throw_TypeError("Array key must be a String or Integer", type, ehi);
}

EH_METHOD(TypeError, initialize);
EH_METHOD(TypeError, toString);

EXTERN_EHLC(TypeError)

// LoopError (when trying to break or continue an invalid number of levels)
void throw_LoopError(const char *msg, int level, EHI *ehi);

EH_METHOD(LoopError, initialize);
EH_METHOD(LoopError, toString);

EXTERN_EHLC(LoopError)

// NameError (when trying to access a non-existent member)
void throw_NameError(ehretval_p object, const char *name, EHI *ehi);

EH_METHOD(NameError, initialize);
EH_METHOD(NameError, toString);

EXTERN_EHLC(NameError)

// ConstError
void throw_ConstError(ehretval_p object, const char *name, EHI *ehi);

EH_METHOD(ConstError, initialize);
EH_METHOD(ConstError, toString);

EXTERN_EHLC(ConstError)

void throw_ArgumentError(const char *message, const char *method, ehretval_p value, EHI *ehi);

static inline void throw_ArgumentError_out_of_range(const char *method, ehretval_p value, EHI *ehi) {
	throw_ArgumentError("Argument out of range", method, value, ehi);
}

EH_METHOD(ArgumentError, initialize);
EH_METHOD(ArgumentError, toString);

EXTERN_EHLC(ArgumentError)

void throw_MiscellaneousError(const char *message, EHI *ehi);

EH_METHOD(MiscellaneousError, initialize);
EH_METHOD(MiscellaneousError, toString);

EXTERN_EHLC(MiscellaneousError)

void throw_SyntaxError(const char *message, int line, EHI *ehi);

EH_METHOD(SyntaxError, initialize);
EH_METHOD(SyntaxError, toString);

EXTERN_EHLC(SyntaxError)

#endif /* _EH_ERROR_H */
