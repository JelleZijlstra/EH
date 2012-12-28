/*
 * EmptyIterator
 * Thrown when an iterator is depleted.
 */
#include <sstream>

#include "EmptyIterator.h"

void throw_EmptyIterator(EHI *ehi) {
	throw_error("EmptyIterator", NULL, ehi);
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
	return ehretval_t::make_resource(obj->get_full_type(), new Exception(strdup("Empty iterator")));
}
