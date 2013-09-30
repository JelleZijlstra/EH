#include "std_lib_includes.hpp"

void throw_UnknownCommandError(const char *msg, EHI *ehi);

EH_METHOD(UnknownCommandError, operator_colon);

EH_INITIALIZER(UnknownCommandError);
