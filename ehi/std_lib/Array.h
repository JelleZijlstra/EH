/*
 * Array class
 */
#include "std_lib_includes.h"

EH_METHOD(Array, initialize);
EH_METHOD(Array, length);
EH_METHOD(Array, has);
EH_METHOD(Array, operator_arrow);
EH_METHOD(Array, operator_arrow_equals);
EH_METHOD(Array, toArray);
EH_METHOD(Array, toTuple);
EH_METHOD(Array, getIterator);

EXTERN_EHLC(Array)

class Array_Iterator : public LibraryBaseClass {
public:
	Array_Iterator(ehretval_p array);
	~Array_Iterator() {}
	bool has_next();
	ehretval_p next(EHI *ehi);
private:
	type_enum current_type;
	eharray_t::string_iterator string_begin;
	eharray_t::string_iterator string_end;
	eharray_t::int_iterator int_begin;
	eharray_t::int_iterator int_end;
	Array_Iterator(const Array_Iterator&);
	Array_Iterator operator=(const Array_Iterator&);
};
EH_METHOD(Array_Iterator, initialize);
EH_METHOD(Array_Iterator, hasNext);
EH_METHOD(Array_Iterator, next);

EXTERN_EHLC(Array_Iterator)
