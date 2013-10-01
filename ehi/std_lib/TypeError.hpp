#include "std_lib_includes.hpp"

#ifndef EH_TYPE_ERROR_H_
#define EH_TYPE_ERROR_H_

[[noreturn]] void throw_TypeError(const char *msg, ehval_p obj, class EHI *ehi);

// common variants
static inline void throw_TypeError_Array_key(ehval_p obj, EHI *ehi) {
	throw_TypeError("Array key must be a String or Integer", obj, ehi);
}

EH_METHOD(TypeError, initialize);

EH_INITIALIZER(TypeError);

template<class T>
void ehval_t::assert_type(const char *method, EHI *ehi) {
	if(!is_a<T>()) {
		std::string message = std::string("Invalid type for argument to ") + method + " (expected " + name<T>() + ")";
		throw_TypeError(message.c_str(), this, ehi);
	}
}

#endif /* EH_TYPE_ERROR_H_ */
