/*
 * Integer class
 */
#include "std_lib_includes.hpp"

#ifndef EH_INTEGER_H_
#define EH_INTEGER_H_

#include <sstream>

EH_CLASS(Integer) {
public:
	typedef int type;
	const int value;

	static ehval_p make(int i) {
		return static_cast<ehval_t *>(new Integer(i));
	}

	virtual bool belongs_in_gc() const {
		return false;
	}

	virtual std::string decompile(int level) {
		std::ostringstream out;
		out << value;
		return out.str();
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) {
		std::cout << "@int " << value << std::endl;
	}
private:
	Integer(int i) : value(i) {}
};

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

EH_CHILD_CLASS(Integer, Iterator) {
private:
	class t {
	public:
		t(int n) : current(0), max(n) {}

		bool has_next();
		int next();
	private:
		int current;
		int max;
	};
	Integer_Iterator(int n) : value(new t(n)) {}

public:
	typedef t *type;
	t *value;

	~Integer_Iterator() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return false;
	}

	static ehval_p make(int n) {
		auto val = new Integer_Iterator(n);
		return static_cast<ehval_t *>(val);
	}
};

EH_METHOD(Integer_Iterator, initialize);
EH_METHOD(Integer_Iterator, hasNext);
EH_METHOD(Integer_Iterator, next);

EH_INITIALIZER(Integer_Iterator);

#endif /* EH_INTEGER_H_ */
