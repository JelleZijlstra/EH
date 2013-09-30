#include "std_lib_includes.hpp"

EH_NORETURN void throw_SyntaxError(const char *message, int line, EHI *ehi);

EH_METHOD(SyntaxError, operator_colon);

EH_INITIALIZER(SyntaxError);
