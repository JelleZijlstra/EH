#include "std_lib_includes.hpp"

void throw_ConstError(ehretval_p object, const char *name, EHI *ehi);

EH_METHOD(ConstError, initialize);

EH_INITIALIZER(ConstError);