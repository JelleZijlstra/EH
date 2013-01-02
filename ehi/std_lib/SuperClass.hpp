#include "std_lib_includes.hpp"

// Superclasses (used for inheritance)
EH_CLASS(SuperClass) {
public:
	typedef ehval_p type;
	type value;

	SuperClass(ehval_p in) : value(in) {}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() {
		return { value };
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) {
		std::cout << "@parent class" << std::endl;
	}

	static ehval_p make(ehval_p super, EHInterpreter *parent);
};

EH_METHOD(SuperClass, toString);

EH_INITIALIZER(SuperClass);
