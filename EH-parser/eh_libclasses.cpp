/*
 * eh_libclasses.cpp
 *
 * EH library classes
 */
#include "eh_libclasses.h"

START_EHLC(CountClass)
EHLC_ENTRY(CountClass, docount)
EHLC_ENTRY(CountClass, setcount)
END_EHLC()

EH_METHOD(CountClass, docount) {
	CountClass *selfptr = (CountClass *)obj;
	*retval = new ehretval_t((int) ++selfptr->count);
}
EH_METHOD(CountClass, setcount) {
	CountClass *selfptr = (CountClass *)obj;

	ehretval_t *args[1];
	if(eh_getargs(paras, 1, args, context, __FUNCTION__, interpreter)) {
		*retval = new ehretval_t(false);
		return;
	}
	ehretval_t *newcounter = eh_xtoint(args[0]);
	if(newcounter->type != int_e) {
		*retval = new ehretval_t(false);
		return;
	}

	selfptr->count = newcounter->intval;
	*retval = new ehretval_t(true);
	return;
}

START_EHLC(File)
EHLC_ENTRY(File, open)
EHLC_ENTRY(File, getc)
EHLC_ENTRY(File, gets)
EHLC_ENTRY(File, puts)
EHLC_ENTRY(File, close)
END_EHLC()

EH_METHOD(File, open) {
	File *selfptr = (File *) obj;

	ehretval_t *args[1];
	if(eh_getargs(paras, 1, args, context, __FUNCTION__, interpreter)) {
		*retval = new ehretval_t(false);
		return;
	}
	ehretval_t *filename = eh_xtostring(args[0]);
	if(filename->type != string_e) {
		*retval = new ehretval_t(false);
		return;
	}
	FILE *mfile = fopen(filename->stringval, "r+");
	if(mfile == NULL) {
		*retval = new ehretval_t(false);
		return;
	}
	selfptr->descriptor = mfile;
	*retval = new ehretval_t(true);
}

EH_METHOD(File, getc) {
	File *selfptr = (File *) obj;

	if(selfptr->descriptor == NULL) {
		return;
	}
	int c = fgetc(selfptr->descriptor);
	if(c == -1) {
		return;
	}
	*retval = new ehretval_t(new char[2]);
	(*retval)->stringval[0] = c;
	(*retval)->stringval[1] = '\0';
}

EH_METHOD(File, gets) {
	File *selfptr = (File *) obj;
	if(selfptr->descriptor == NULL) {
		return;
	}
	
	*retval = new ehretval_t(new char[512]);
	
	char *ptr = fgets((*retval)->stringval, 511, selfptr->descriptor);
	if(ptr == NULL) {
		delete[] (*retval)->stringval;
		delete *retval;
		*retval = NULL;
	}
}

EH_METHOD(File, puts) {
	File *selfptr = (File *) obj;
	if(selfptr->descriptor == NULL) {
		return;
	}
	
	ehretval_t *args[1];
	if(eh_getargs(paras, 1, args, context, __FUNCTION__, interpreter)) {
		*retval = new ehretval_t(false);
		return;
	}
	ehretval_t *str = eh_xtostring(args[0]);
	if(str->type != string_e) {
		*retval = new ehretval_t(false);
		return;
	}

	int ret = fputs(str->stringval, selfptr->descriptor);
	
	if(ret == EOF) {
		*retval = new ehretval_t(false);
	} else {
		*retval = new ehretval_t(true);
	}
}

EH_METHOD(File, close) {
	File *selfptr = (File *) obj;
	if(selfptr->descriptor == NULL) {
		return;
	}
	fclose(selfptr->descriptor);
	selfptr->descriptor = NULL;
}
