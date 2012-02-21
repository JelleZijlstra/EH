/*
 * eh_libclasses.h
 *
 * Header file for EH library classes
 */
#include "eh.h"

#define EHLC_CONSTRUCTOR(name) inline void *ehlc_new_ ## name() { \
	return (void *)new name(); \
}
#define START_EHLC(name) ehlibentry_t ehlc_l_ ## name [] = {
#define EHLC_ENTRY(classn, name) { #name, &ehlm_ ## classn ## _ ## name },
#define END_EHLC() {NULL, NULL} };
#define EXTERN_EHLC(name) extern ehlibentry_t ehlc_l_ ## name [];

#define EH_METHOD(classn,name) void ehlm_ ## classn ## _ ## name(void *obj, ehretval_t *paras, ehretval_t *retval, ehcontext_t context)


/*
 * CountClass library class
 */
class CountClass {
public:
	int count;
	CountClass() {
		count = 0;
	}
};
EH_METHOD(CountClass, docount);
EH_METHOD(CountClass, setcount);

EHLC_CONSTRUCTOR(CountClass)
EXTERN_EHLC(CountClass)

/*
 * File library class
 */
class File {
public:
	FILE *descriptor;
	File() {
		descriptor = NULL;
	}
};
EH_METHOD(File, open);
EH_METHOD(File, getc);
EH_METHOD(File, gets);
EH_METHOD(File, close);

EHLC_CONSTRUCTOR(File)
EXTERN_EHLC(File)
