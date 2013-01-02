#include "std_lib_includes.hpp"

void throw_ConstError(ehval_p object, const char *name, EHI *ehi);

EH_METHOD(ConstError, initialize);

EH_INITIALIZER(ConstError);
