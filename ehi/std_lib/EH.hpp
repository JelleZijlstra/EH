#ifndef EH_EH_H_
#define EH_EH_H_

#include "std_lib_includes.hpp"

EH_INITIALIZER(EH);

EH_METHOD(EH, eval);
EH_METHOD(EH, contextName);
EH_METHOD(EH, collectGarbage);
EH_METHOD(EH, parse);

#endif /* EH_EH_H_ */
