/*
 * EmptyIterator
 * Thrown when an iterator is depleted.
 */
#include <sstream>

#include "EmptyIterator.hpp"

void throw_EmptyIterator(EHI *ehi) {
	throw_error("EmptyIterator", nullptr, ehi);
}

EH_INITIALIZER(EmptyIterator) {
	REGISTER_METHOD(EmptyIterator, initialize);
	INHERIT_LIBRARY(Exception);
}

/*
 * @description Initializer. Takes no arguments.
 * @argument None
 * @returns N/A
 */
EH_METHOD(EmptyIterator, initialize) {
	return Exception::make(strdup("Empty iterator"));
}
