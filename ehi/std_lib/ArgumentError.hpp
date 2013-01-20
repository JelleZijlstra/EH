#ifndef EH_ARGUMENT_ERROR_H_
#define EH_ARGUMENT_ERROR_H_

#include "std_lib_includes.hpp"

void throw_ArgumentError(const char *message, const char *method, ehval_p value, EHI *ehi) [[noreturn]];

static inline void throw_ArgumentError_out_of_range(const char *method, ehval_p value, EHI *ehi) [[noreturn]] {
	throw_ArgumentError("Argument out of range", method, value, ehi);
}

EH_METHOD(ArgumentError, initialize);
EH_METHOD(ArgumentError, toString);

EH_INITIALIZER(ArgumentError);

#endif /* EH_ARGUMENT_ERROR_H_ */
