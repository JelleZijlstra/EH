#include "std_lib_includes.h"

void throw_MiscellaneousError(const char *message, EHI *ehi);

EH_METHOD(MiscellaneousError, initialize);
EH_METHOD(MiscellaneousError, toString);

EXTERN_EHLC(MiscellaneousError)