// VisibilityError: thrown when accessing a private member from an inappropriate context
#include "std_lib_includes.hpp"

[[noreturn]] void throw_VisibilityError(ehval_p object, const char *name, EHI *ehi);

EH_METHOD(VisibilityError, operator_colon);

EH_INITIALIZER(VisibilityError);
