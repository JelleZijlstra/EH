/*
* String class
*/
#include "std_lib_includes.h"

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
EH_METHOD(String, getIterator);

EH_INITIALIZER(String);

class String_Iterator : public LibraryBaseClass {
public:
	String_Iterator(ehretval_p _string) : string(_string), position(0) {}
	~String_Iterator() {}
	bool has_next() const;
	char next();
	char peek() const;
private:
	ehretval_p string;
	size_t position;
	String_Iterator(const String_Iterator&);
	String_Iterator operator=(const String_Iterator&);
};
EH_METHOD(String_Iterator, initialize);
EH_METHOD(String_Iterator, hasNext);
EH_METHOD(String_Iterator, next);
EH_METHOD(String_Iterator, peek);

EH_INITIALIZER(String_Iterator);
