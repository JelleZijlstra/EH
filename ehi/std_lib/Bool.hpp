#include "std_lib_includes.hpp"

#ifndef EH_BOOL_H_
#define EH_BOOL_H_
/*
 * Bool class
 */

EH_CLASS(Bool) {
public:
	typedef bool type;
	const bool value;

	static ehval_p make(bool b) {
		// TODO: cache true and false objects here
		return static_cast<ehval_t *>(new Bool(b));
	}

	virtual bool belongs_in_gc() const {
		return false;
	}

	virtual std::string decompile(int level) {
		return value ? "true" : "false";
	}

	virtual void printvar(printvar_set &, int level, EHI *ehi) {
		std::cout << (value ? "@bool true" : "@bool false") << std::endl;
	}
private:
	Bool(bool b) : value(b) {}
};

EH_METHOD(Bool, initialize);
EH_METHOD(Bool, compare);
EH_METHOD(Bool, toString);
EH_METHOD(Bool, toBool);
EH_METHOD(Bool, toInteger);
EH_METHOD(Bool, operator_bang);

EH_INITIALIZER(Bool);

#endif /* EH_BOOL_H_ */
