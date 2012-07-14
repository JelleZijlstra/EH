/*
 * eh_libclasses.h
 *
 * Header file for EH library classes
 */
#include "eh.h"

#define EHLC_CONSTRUCTOR_DESTRUCTOR(name) inline void *ehlc_new_ ## name() { \
	return (void *)new name(); \
} \
inline void ehlc_delete_ ## name(ehobj_t *in) { \
	delete (name *)in->object_data->get_resourceval(); \
}
#define START_EHLC(name) ehlm_listentry_t ehlc_l_ ## name [] = {
#define EHLC_ENTRY(classn, name) { #name, &ehlm_ ## classn ## _ ## name },
#define END_EHLC() {NULL, NULL} };
#define EXTERN_EHLC(name) extern ehlm_listentry_t ehlc_l_ ## name [];

#define EH_METHOD(classn,name) ehretval_p ehlm_ ## classn ## _ ## name(ehretval_p obj, int nargs, ehretval_p *args, ehcontext_t context, EHI *interpreter)

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

EHLC_CONSTRUCTOR_DESTRUCTOR(CountClass)
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
EH_METHOD(File, puts);
EH_METHOD(File, close);

EHLC_CONSTRUCTOR_DESTRUCTOR(File)
EXTERN_EHLC(File)

/*
 * Integer class
 */
EH_METHOD(Integer, operator_plus);
EH_METHOD(Integer, abs);

EXTERN_EHLC(Integer)

/*
 * Array class
 */
EH_METHOD(Array, count);
EH_METHOD(Array, operator_arrow);
EH_METHOD(Array, operator_arrow_equals);

EXTERN_EHLC(Array)

/*
 * Float class
 */
EH_METHOD(Float, operator_plus);
EH_METHOD(Float, abs);

/*
 * String class
 */
EH_METHOD(String, length);
EH_METHOD(String, operator_arrow);
EH_METHOD(String, operator_arrow_equals);
EH_METHOD(String, operator_plus);
