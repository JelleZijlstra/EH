/*
 * Exception class
 */
#ifndef EH_EXCEPTION_H_
#define EH_EXCEPTION_H_

#include "std_lib_includes.h"
 
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

#endif /* EH_EXCEPTION_H_ */