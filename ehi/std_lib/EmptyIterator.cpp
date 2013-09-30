/*
 * EmptyIterator
 * Thrown when an iterator is depleted.
 */
#include "EmptyIterator.hpp"

#include "Exception.hpp"

void throw_EmptyIterator(EHI *ehi) {
	throw_error("EmptyIterator", nullptr, ehi);
}

EH_INITIALIZER(EmptyIterator) {
	REGISTER_METHOD(EmptyIterator, operator_colon);
	INHERIT_LIBRARY(Exception);
}

/*
 * @description Initializer. Takes no arguments.
 * @argument None
 * @returns N/A
 */
EH_METHOD(EmptyIterator, operator_colon) {
	return Exception::make(strdup("Empty iterator"));
}
