/*
 * Exception class
 */
#ifndef EH_EXCEPTION_H_
#define EH_EXCEPTION_H_

#include "std_lib_includes.hpp"

EH_CLASS(Exception) {
public:
	typedef const char *type;
	const char *value;

	Exception(const char *msg) : value(msg) {}
	virtual ~Exception() {
		delete[] value;
	}

	virtual bool belongs_in_gc() const {
		return false;
	}

	static ehval_p make(const char *value) {
		return static_cast<ehval_t*>(new Exception(value));
	}

private:
	Exception(const Exception&);
	Exception operator=(const Exception&);
};
EH_METHOD(Exception, toString);
EH_METHOD(Exception, initialize);

EH_INITIALIZER(Exception);

#endif /* EH_EXCEPTION_H_ */
