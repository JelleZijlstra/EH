// NameError (when trying to access a non-existent member)
#include "std_lib_includes.h"

void throw_NameError(ehretval_p object, const char *name, EHI *ehi);

EH_METHOD(NameError, initialize);
EH_METHOD(NameError, toString);

EXTERN_EHLC(NameError)