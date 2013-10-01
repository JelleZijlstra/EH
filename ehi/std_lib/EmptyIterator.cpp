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
	REGISTER_METHOD(EmptyIterator, initialize);
	INHERIT_PURE_CLASS(Exception);
}

/*
 * @description Initializer. Takes no arguments.
 * @argument None
 * @returns N/A
 */
EH_METHOD(EmptyIterator, initialize) {
	obj->set_property("message", String::make(strdup("Empty iterator")), ehi->global(), ehi);
	return nullptr;
}
