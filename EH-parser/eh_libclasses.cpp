/*
 * eh_libclasses.cpp
 *
 * EH library classes
 */
#include "eh_libclasses.h"

START_EHLC(CountClass)
EHLC_ENTRY(CountClass, docount)
END_EHLC()

EH_METHOD(CountClass, docount) {
	CountClass *selfptr = (CountClass *)obj;
	retval->type = int_e;
	retval->intval = ++selfptr->count;
}
