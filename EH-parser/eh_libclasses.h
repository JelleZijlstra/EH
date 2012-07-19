/*
 * eh_libclasses.h
 *
 * Header file for EH library classes
 */
#include "eh.h"

#define START_EHLC(name) ehlm_listentry_t ehlc_l_ ## name [] = {
#define EHLC_ENTRY(classn, name) { #name, &ehlm_ ## classn ## _ ## name },
#define END_EHLC() {NULL, NULL} };
#define EXTERN_EHLC(name) extern ehlm_listentry_t ehlc_l_ ## name [];

#define EH_METHOD(classn, name) ehretval_p ehlm_ ## classn ## _ ## name(ehretval_p obj, int nargs, ehretval_p *args, ehcontext_t context, EHI *ehi)

class LibraryBaseClass {
public:
  virtual ~LibraryBaseClass() {};
};

/*
 * Object class
 */
EH_METHOD(Object, new);
EH_METHOD(Object, initialize);
EH_METHOD(Object, toString);
EH_METHOD(Object, finalize);

EXTERN_EHLC(Object)

/*
 * CountClass library class
 */
class CountClass : public LibraryBaseClass {
public:
	int count;
	CountClass() : count(0) {}
	~CountClass() {}
};
EH_METHOD(CountClass, initialize);
EH_METHOD(CountClass, docount);
EH_METHOD(CountClass, setcount);

EXTERN_EHLC(CountClass)

/*
 * File library class
 */
class File : public LibraryBaseClass {
public:
	FILE *descriptor;
	File() : descriptor(NULL) {}
	~File() {}
};
EH_METHOD(File, initialize);
EH_METHOD(File, open);
EH_METHOD(File, getc);
EH_METHOD(File, gets);
EH_METHOD(File, puts);
EH_METHOD(File, close);
EH_METHOD(File, toBool);
EH_METHOD(File, finalize);

EXTERN_EHLC(File)

/*
 * Integer class
 */
EH_METHOD(Integer, operator_plus);
EH_METHOD(Integer, operator_minus);
EH_METHOD(Integer, operator_times);
EH_METHOD(Integer, operator_divide);
EH_METHOD(Integer, operator_modulo);
EH_METHOD(Integer, abs);
EH_METHOD(Integer, getBit);
EH_METHOD(Integer, setBit);
EH_METHOD(Integer, length);
EH_METHOD(Integer, toString);
EH_METHOD(Integer, toBool);
EH_METHOD(Integer, toFloat);
EH_METHOD(Integer, toInt);
EH_METHOD(Integer, sqrt);

EXTERN_EHLC(Integer)

/*
 * Array class
 */
EH_METHOD(Array, length);
EH_METHOD(Array, operator_arrow);
EH_METHOD(Array, operator_arrow_equals);

EXTERN_EHLC(Array)

/*
 * Float class
 */
EH_METHOD(Float, operator_plus);
EH_METHOD(Float, operator_minus);
EH_METHOD(Float, operator_times);
EH_METHOD(Float, operator_divide);
EH_METHOD(Float, abs);
EH_METHOD(Float, toString);
EH_METHOD(Float, toInt);
EH_METHOD(Float, toBool);
EH_METHOD(Float, toFloat);
EH_METHOD(Float, sqrt);

EXTERN_EHLC(Float)

/*
 * String class
 */
EH_METHOD(String, length);
EH_METHOD(String, operator_arrow);
EH_METHOD(String, operator_arrow_equals);
EH_METHOD(String, operator_plus);
EH_METHOD(String, toString);
EH_METHOD(String, toInt);
EH_METHOD(String, toFloat);
EH_METHOD(String, toBool);
EH_METHOD(String, toRange);
EH_METHOD(String, charAtPosition);

EXTERN_EHLC(String)

/*
 * Bool class
 */
EH_METHOD(Bool, toString);
EH_METHOD(Bool, toBool);
EH_METHOD(Bool, toInt);

EXTERN_EHLC(Bool)

/*
 * Null class
 */
EH_METHOD(Null, toString);
EH_METHOD(Null, toBool);

EXTERN_EHLC(Null)

/*
 * Range class
 */
EH_METHOD(Range, operator_arrow);
EH_METHOD(Range, min);
EH_METHOD(Range, max);
EH_METHOD(Range, toString);
EH_METHOD(Range, toArray);
EH_METHOD(Range, toRange);

EXTERN_EHLC(Range)

/*
 * Hash class
 */
EH_METHOD(Hash, toArray);
EH_METHOD(Hash, operator_arrow);
EH_METHOD(Hash, operator_arrow_equals);
EH_METHOD(Hash, has);

EXTERN_EHLC(Hash)

/*
 * Function class
 */
EH_METHOD(Function, operator_colon);

EXTERN_EHLC(Function)
