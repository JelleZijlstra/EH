/*
 * CountClass library class
 */
#include "std_lib_includes.h"

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
