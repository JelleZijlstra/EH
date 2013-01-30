#include "std_lib_includes.hpp"

[[noreturn]] void throw_SyntaxError(const char *message, int line, EHI *ehi);

EH_METHOD(SyntaxError, initialize);

EH_INITIALIZER(SyntaxError);
