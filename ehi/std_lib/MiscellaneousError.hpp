#include "std_lib_includes.hpp"

EH_NORETURN void throw_MiscellaneousError(const char *message, EHI *ehi);

EH_METHOD(MiscellaneousError, operator_colon);

EH_INITIALIZER(MiscellaneousError);
