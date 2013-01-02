/*
 * Null class
 */
#include "std_lib_includes.hpp"

#ifndef EH_NULL_H_
#define EH_NULL_H_

EH_CLASS(Null) {
public:
	static ehval_p null_obj;

	virtual bool belongs_in_gc() const {
		return false;
	}

	virtual std::string decompile(int level) {
		return "()";
	}

	virtual void printvar(printvar_set& set, int level, EHI *ehi) {
		std::cout << "null" << std::endl;
	}

	static ehval_p make() {
		return null_obj;
	}
};

EH_METHOD(Null, initialize);
EH_METHOD(Null, toString);
EH_METHOD(Null, toBool);

EH_INITIALIZER(Null);

#endif /* EH_NULL_H_ */
