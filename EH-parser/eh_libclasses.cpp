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

START_EHLC(File)
EHLC_ENTRY(File, open)
EHLC_ENTRY(File, getc)
EHLC_ENTRY(File, close)
END_EHLC()

EH_METHOD(File, open) {
	File *selfptr = (File *) obj;
	retval->type = bool_e;
	
	ehretval_t *args = (ehretval_t *) Malloc(1 * sizeof(ehretval_t));
	if(eh_getargs(paras, 1, args, context, __FUNCTION__)) {
		retval->boolval = false;
		return;
	}
	ehretval_t filename = eh_xtostring(args[0]);
	if(filename.type != string_e) {
		retval->boolval = false;
		return;
	}
	FILE *mfile = fopen(filename.stringval, "r");
	if(mfile == NULL) {
		retval->boolval = false;
		return;
	}
	selfptr->descriptor = mfile;
	retval->boolval = true;
}

EH_METHOD(File, getc) {
	File *selfptr = (File *) obj;

	if(selfptr->descriptor == NULL) {
		retval->type = null_e;
		return;
	}
	int c = fgetc(selfptr->descriptor);
	if(c == -1) {
		retval->type = null_e;
		return;
	}
	retval->type = string_e;
	retval->stringval = new char[2];
	retval->stringval[0] = c;
	retval->stringval[1] = '\0';
}

EH_METHOD(File, close) {
	File *selfptr = (File *) obj;
	if(selfptr->descriptor == NULL) {
		return;
	}
	fclose(selfptr->descriptor);
	selfptr->descriptor = NULL;
}