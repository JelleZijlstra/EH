#ifndef EH_EMPTY_ITERATOR_H_
#define EH_EMPTY_ITERATOR_H_

#include "std_lib_includes.hpp"

void throw_EmptyIterator(EHI *ehi);

EH_METHOD(EmptyIterator, initialize);

EH_INITIALIZER(EmptyIterator);

#endif /* EH_EMPTY_ITERATOR_H_ */
