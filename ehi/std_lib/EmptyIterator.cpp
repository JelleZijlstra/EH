#include <sstream>

#include "EmptyIterator.h"

void throw_EmptyIterator(EHI *ehi) {
	throw_error("EmptyIterator", NULL, ehi);
}

EH_INITIALIZER(EmptyIterator) {
	REGISTER_METHOD(EmptyIterator, initialize);
	INHERIT_LIBRARY(Exception);
}

EH_METHOD(EmptyIterator, initialize) {
	return ehretval_t::make_resource(new Exception(strdup("Empty iterator")));	
}
