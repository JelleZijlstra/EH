#ifndef EH_TYPE_ERROR_H_
#define EH_TYPE_ERROR_H_

void throw_TypeError(const char *msg, int type, class EHI *ehi);

#include "std_lib_includes.hpp"

// common variants
static inline void throw_TypeError_Array_key(int type, EHI *ehi) {
	throw_TypeError("Array key must be a String or Integer", type, ehi);
}

EH_METHOD(TypeError, initialize);

EH_INITIALIZER(TypeError);

#endif /* EH_TYPE_ERROR_H_ */
