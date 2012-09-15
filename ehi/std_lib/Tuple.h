/*
 * Tuple class
 */
#include "std_lib_includes.h"

EH_METHOD(Tuple, initialize);
EH_METHOD(Tuple, operator_arrow);
EH_METHOD(Tuple, length);
EH_METHOD(Tuple, toTuple);
EH_METHOD(Tuple, getIterator);

EXTERN_EHLC(Tuple)

class Tuple_Iterator : public LibraryBaseClass {
public:
	Tuple_Iterator(ehretval_p _tuple) : tuple(_tuple), position(0) {}
	~Tuple_Iterator() {}
	bool has_next();
	ehretval_p next();
private:
	ehretval_p tuple;
	int position;
	Tuple_Iterator(const Tuple_Iterator &);
	Tuple_Iterator operator=(const Tuple_Iterator &);
};

EH_METHOD(Tuple_Iterator, initialize);
EH_METHOD(Tuple_Iterator, hasNext);
EH_METHOD(Tuple_Iterator, next);

EXTERN_EHLC(Tuple_Iterator)
