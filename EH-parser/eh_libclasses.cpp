/*
 * eh_libclasses.cpp
 *
 * EH library classes
 */
#include "eh_libclasses.h"
#include "eh_error.h"

START_EHLC(CountClass)
EHLC_ENTRY(CountClass, docount)
EHLC_ENTRY(CountClass, setcount)
END_EHLC()

EH_METHOD(CountClass, docount) {
	if(nargs != 0) {
		eh_error_argcount_lib("CountClass::docount", 0, nargs);
		return NULL;
	}
	CountClass *selfptr = (CountClass *)obj;
	return ehretval_t::make_int(++selfptr->count);
}
EH_METHOD(CountClass, setcount) {
	if(nargs != 1) {
		eh_error_argcount_lib("CountClass::setcount", 1, nargs);
		return NULL;
	}
	CountClass *selfptr = (CountClass *)obj;

	ehretval_p newcounter = eh_xtoint(args[0]);
	if(newcounter->type() != int_e) {
		return NULL;
	}

	selfptr->count = newcounter->intval;
	return ehretval_t::make_bool(true);
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

	if(nargs != 1) {
		eh_error_argcount_lib("File::open", 1, nargs);
		return NULL;
	}
	ehretval_p filename = eh_xtostring(args[0]);
	if(filename->type() != string_e) {
		return NULL;
	}
	FILE *mfile = fopen(filename->stringval, "r+");
	if(mfile == NULL) {
		return NULL;
	}
	selfptr->descriptor = mfile;
	return ehretval_t::make_bool(true);
}

EH_METHOD(File, getc) {
	File *selfptr = (File *) obj;
	if(nargs != 0) {
		eh_error_argcount_lib("File::getc", 0, nargs);
		return NULL;
	}

	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	int c = fgetc(selfptr->descriptor);
	if(c == -1) {
		return NULL;
	}
	char *out = new char[2];
	out[0] = c;
	out[1] = '\0';
	return ehretval_t::make_string(out);
}

EH_METHOD(File, gets) {
	File *selfptr = (File *) obj;
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	if(nargs != 0) {
		eh_error_argcount_lib("File::gets", 0, nargs);
		return NULL;
	}
	
	char *out = new char[512];

	char *ptr = fgets(out, 511, selfptr->descriptor);
	if(ptr == NULL) {
		delete[] out;
		return NULL;
	}
	return ehretval_t::make_string(out);
}

EH_METHOD(File, puts) {
	File *selfptr = (File *) obj;
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	
	if(nargs != 1) {
		eh_error_argcount_lib("File::puts", 1, nargs);
		return NULL;
	}
	ehretval_p str = eh_xtostring(args[0]);
	if(str->type() != string_e) {
		return NULL;
	}

	int count = fputs(str->stringval, selfptr->descriptor);
	
	if(count == EOF) {
		return ehretval_t::make_bool(false);
	} else {
		return ehretval_t::make_bool(true);
	}
}

EH_METHOD(File, close) {
	File *selfptr = (File *) obj;
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	if(nargs != 0) {
		eh_error_argcount_lib("File::close", 0, nargs);
		return NULL;
	}

	fclose(selfptr->descriptor);
	selfptr->descriptor = NULL;
	return NULL;
}
