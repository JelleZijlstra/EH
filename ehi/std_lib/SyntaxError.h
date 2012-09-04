#include "std_lib_includes.h"

void throw_SyntaxError(const char *message, int line, EHI *ehi);

EH_METHOD(SyntaxError, initialize);
EH_METHOD(SyntaxError, toString);

EXTERN_EHLC(SyntaxError)
