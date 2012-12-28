#ifndef EH_ERROR_H_
#define EH_ERROR_H_

#include "eh_libclasses.hpp"

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

#endif /* EH_ERROR_H_ */
