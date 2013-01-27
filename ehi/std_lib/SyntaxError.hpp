#include "std_lib_includes.hpp"

void throw_SyntaxError(const char *message, int line, EHI *ehi) [[noreturn]];

EH_METHOD(SyntaxError, initialize);

EH_INITIALIZER(SyntaxError);
