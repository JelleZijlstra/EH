// NameError (when trying to access a non-existent member)
#include "std_lib_includes.hpp"

[[noreturn]] void throw_NameError(ehval_p object, const char *name, EHI *ehi);

EH_METHOD(NameError, initialize);

EH_INITIALIZER(NameError);
