// VisibilityError: thrown when accessing a private member from an inappropriate context
#include "std_lib_includes.h"

void throw_VisibilityError(ehretval_p object, const char *name, EHI *ehi);

EH_METHOD(VisibilityError, initialize);

EH_INITIALIZER(VisibilityError);
