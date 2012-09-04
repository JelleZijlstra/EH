#include "std_lib_includes.h"

void throw_UnknownCommandError(const char *msg, EHI *ehi);

EH_METHOD(UnknownCommandError, initialize);
EH_METHOD(UnknownCommandError, toString);

EXTERN_EHLC(UnknownCommandError)
