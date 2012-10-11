/*
 * Integer class
 */
#include "std_lib_includes.h"

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
EH_METHOD(Integer, operator_leftshift);
EH_METHOD(Integer, operator_rightshift);
EH_METHOD(Integer, compare);
EH_METHOD(Integer, abs);
EH_METHOD(Integer, getBit);
EH_METHOD(Integer, setBit);
EH_METHOD(Integer, length);
EH_METHOD(Integer, toString);
EH_METHOD(Integer, toBool);
EH_METHOD(Integer, toFloat);
EH_METHOD(Integer, toChar);
EH_METHOD(Integer, toInt);
EH_METHOD(Integer, sqrt);
EH_METHOD(Integer, getIterator);

EH_INITIALIZER(Integer);

class Integer_Iterator : public LibraryBaseClass {
public:
	Integer_Iterator(int n) : current(0), max(n) {}
	~Integer_Iterator() {}

	bool has_next();
	int next();
private:
	int current;
	int max;
	Integer_Iterator(const Integer_Iterator&);
	Integer_Iterator operator=(const Integer_Iterator&);
};

EH_METHOD(Integer_Iterator, initialize);
EH_METHOD(Integer_Iterator, hasNext);
EH_METHOD(Integer_Iterator, next);

EH_INITIALIZER(Integer_Iterator);

