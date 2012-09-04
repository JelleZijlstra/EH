// LoopError (when trying to break or continue an invalid number of levels)
#include "std_lib_includes.h"

void throw_LoopError(const char *msg, int level, EHI *ehi);

EH_METHOD(LoopError, initialize);
EH_METHOD(LoopError, toString);

EXTERN_EHLC(LoopError)
