/*
 * eh_libclasses.h
 *
 * Header file for EH library classes
 */
#include "eh.h"

#ifndef _EH_LIBCLASSES_H
#define _EH_LIBCLASSES_H

#define START_EHLC(name) ehlm_listentry_t ehlc_l_ ## name [] = {
#define EHLC_ENTRY(classn, name) { #name, &ehlm_ ## classn ## _ ## name },
#define EHLC_ENTRY_RENAME(classn, name, user_name) { user_name, &ehlm_ ## classn ## _ ## name },
#define END_EHLC() {NULL, NULL} };
#define EXTERN_EHLC(name) extern ehlm_listentry_t ehlc_l_ ## name [];

#define EH_METHOD(classn, name) ehretval_p ehlm_ ## classn ## _ ## name(ehretval_p obj, ehretval_p args, ehcontext_t context, EHI *ehi)

#define ASSERT_TYPE(operand, ehtype, method) if(operand->type() != ehtype) { \
	throw_TypeError("Invalid type for argument to " method, operand->type(), ehi); \
}
#define ASSERT_NARGS(count, method) ASSERT_TYPE(args, tuple_e, method); \
	if(args->get_tupleval()->size() != count) { \
		throw_ArgumentError("Argument must be a tuple of size " #count, method, args, ehi); \
	}
#define ASSERT_OBJ_TYPE(ehtype, method) if(obj->type() != ehtype) { \
	throw_TypeError("Invalid base object for " #method, obj->type(), ehi); \
}
#define ASSERT_NULL(method) if(args->type() != null_e) { \
	throw_TypeError("Argument to " #method " must be null", args->type(), ehi); \
}
#define ASSERT_NARGS_AND_TYPE(count, ehtype, method) ASSERT_NARGS(count, method); ASSERT_OBJ_TYPE(ehtype, method);
#define ASSERT_NULL_AND_TYPE(ehtype, method) ASSERT_NULL(method); ASSERT_OBJ_TYPE(ehtype, method);


class LibraryBaseClass {
public:
  virtual ~LibraryBaseClass() {}
};

/*
 * Object class
 */
EH_METHOD(Object, new);
EH_METHOD(Object, inherit);
EH_METHOD(Object, initialize);
EH_METHOD(Object, toString);
EH_METHOD(Object, finalize);
EH_METHOD(Object, isA);
EH_METHOD(Object, operator_compare);
EH_METHOD(Object, compare);
EH_METHOD(Object, operator_equals);
EH_METHOD(Object, operator_ne);
EH_METHOD(Object, operator_gt);
EH_METHOD(Object, operator_gte);
EH_METHOD(Object, operator_lt);
EH_METHOD(Object, operator_lte);

EXTERN_EHLC(Object)

/*
 * GlobalObject
 */
EH_METHOD(GlobalObject, toString);

EXTERN_EHLC(GlobalObject)

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
private:
	File(const File&);
	File operator=(const File&);
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
EH_METHOD(Integer, initialize);
EH_METHOD(Integer, operator_plus);
EH_METHOD(Integer, operator_minus);
EH_METHOD(Integer, operator_times);
EH_METHOD(Integer, operator_divide);
EH_METHOD(Integer, operator_modulo);
EH_METHOD(Integer, operator_and);
EH_METHOD(Integer, operator_or);
EH_METHOD(Integer, operator_xor);
EH_METHOD(Integer, operator_tilde);
EH_METHOD(Integer, operator_uminus);
EH_METHOD(Integer, compare);
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
EH_METHOD(Array, initialize);
EH_METHOD(Array, length);
EH_METHOD(Array, operator_arrow);
EH_METHOD(Array, operator_arrow_equals);
EH_METHOD(Array, toArray);
EH_METHOD(Array, toTuple);

EXTERN_EHLC(Array)

/*
 * Float class
 */
EH_METHOD(Float, initialize);
EH_METHOD(Float, operator_plus);
EH_METHOD(Float, operator_minus);
EH_METHOD(Float, operator_times);
EH_METHOD(Float, operator_divide);
EH_METHOD(Float, operator_uminus);
EH_METHOD(Float, compare);
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
EH_METHOD(String, initialize);
EH_METHOD(String, length);
EH_METHOD(String, operator_arrow);
EH_METHOD(String, operator_arrow_equals);
EH_METHOD(String, operator_plus);
EH_METHOD(String, compare);
EH_METHOD(String, operator_equals);
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
EH_METHOD(Bool, initialize);
EH_METHOD(Bool, toString);
EH_METHOD(Bool, toBool);
EH_METHOD(Bool, toInt);
EH_METHOD(Bool, operator_bang);

EXTERN_EHLC(Bool)

/*
 * Null class
 */
EH_METHOD(Null, initialize);
EH_METHOD(Null, toString);
EH_METHOD(Null, toBool);

EXTERN_EHLC(Null)

/*
 * Range class
 */
EH_METHOD(Range, initialize);
EH_METHOD(Range, operator_arrow);
EH_METHOD(Range, min);
EH_METHOD(Range, max);
EH_METHOD(Range, toString);
EH_METHOD(Range, toArray);
EH_METHOD(Range, toRange);
EH_METHOD(Range, compare);

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
EH_METHOD(Function, toString);

EXTERN_EHLC(Function)

/*
 * Exception class
 */
class Exception : public LibraryBaseClass {
public:
	const char *msg;
	Exception(const char *_msg) : msg(_msg) {}
	virtual ~Exception() {
		delete[] msg;
	}
private:
	Exception(const Exception&);
	Exception operator=(const Exception&);
};
EH_METHOD(Exception, toString);
EH_METHOD(Exception, initialize);

EXTERN_EHLC(Exception)

/*
 * Tuple class
 */
EH_METHOD(Tuple, initialize);
EH_METHOD(Tuple, operator_arrow);
EH_METHOD(Tuple, length);
EH_METHOD(Tuple, toTuple);

EXTERN_EHLC(Tuple)

/*
 * GarbageCollector class
 */
EH_METHOD(GarbageCollector, run);
EH_METHOD(GarbageCollector, stats);

EXTERN_EHLC(GarbageCollector)

#endif /* _EH_LIBCLASSES_H */
