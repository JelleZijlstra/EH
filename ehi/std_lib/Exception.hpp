/*
 * Exception class
 */
#include "std_lib_includes.hpp"

#ifndef EH_EXCEPTION_H_
#define EH_EXCEPTION_H_

EH_NORETURN void throw_error(const char *class_name, ehval_p args, EHI *ehi);

EH_METHOD(Exception, toString);
EH_METHOD(Exception, initialize);

EH_INITIALIZER(Exception);

#endif /* EH_EXCEPTION_H_ */
