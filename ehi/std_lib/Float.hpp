#include "std_lib_includes.hpp"

#ifndef EH_FLOAT_H_
#define EH_FLOAT_H_
/*
 * Float class
 */

#include <sstream>

EH_CLASS(Float) {
public:
	typedef float type;
	const float value;

	static ehval_p make(float f) {
		return static_cast<ehval_t *>(new Float(f));
	}

	virtual bool belongs_in_gc() const {
		return false;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		std::cout << "@float " << value << std::endl;
	}

	virtual std::string decompile(int level) const override {
		std::ostringstream out;
		out << value;
		return out.str();
	}
private:
	Float(float f) : value(f) {}
};

EH_METHOD(Float, initialize);
EH_METHOD(Float, operator_plus);
EH_METHOD(Float, operator_minus);
EH_METHOD(Float, operator_times);
EH_METHOD(Float, operator_divide);
EH_METHOD(Float, compare);
EH_METHOD(Float, abs);
EH_METHOD(Float, toString);
EH_METHOD(Float, toInteger);
EH_METHOD(Float, toBool);
EH_METHOD(Float, toFloat);
EH_METHOD(Float, sqrt);
EH_METHOD(Float, round);

EH_INITIALIZER(Float);

#endif /* EH_FLOAT_H_ */
