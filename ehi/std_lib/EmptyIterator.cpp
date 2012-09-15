#include <sstream>

#include "EmptyIterator.h"

void throw_EmptyIterator(EHI *ehi) {
	throw_error("EmptyIterator", NULL, ehi);
}

START_EHLC(EmptyIterator)
	EHLC_ENTRY(EmptyIterator, initialize)
	INHERIT_LIBRARY(Exception);
END_EHLC()

EH_METHOD(EmptyIterator, initialize) {
	return ehretval_t::make_resource(new Exception(strdup("Empty iterator")));	
}
