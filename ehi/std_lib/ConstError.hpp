#include "std_lib_includes.hpp"

[[noreturn]] void throw_ConstError(ehval_p object, const char *name, EHI *ehi);

EH_METHOD(ConstError, operator_colon);

EH_INITIALIZER(ConstError);
