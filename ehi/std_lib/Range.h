/*
 * Range class
 */
#include "std_lib_includes.h"

EH_METHOD(Range, initialize);
EH_METHOD(Range, operator_arrow);
EH_METHOD(Range, min);
EH_METHOD(Range, max);
EH_METHOD(Range, toString);
EH_METHOD(Range, toArray);
EH_METHOD(Range, toRange);
EH_METHOD(Range, compare);
EH_METHOD(Range, getIterator);

EXTERN_EHLC(Range)

class Range_Iterator : public LibraryBaseClass {
public:
	Range_Iterator(ehretval_p _range) : range(_range), current(this->range->get_rangeval()->min) {}
	~Range_Iterator() {}
	bool has_next(EHI *ehi);
	ehretval_p next(EHI *ehi);
private:
	ehretval_p range;
	ehretval_p current;
	Range_Iterator(const Range_Iterator&);
	Range_Iterator operator=(const Range_Iterator&);
};
EH_METHOD(Range_Iterator, initialize);
EH_METHOD(Range_Iterator, hasNext);
EH_METHOD(Range_Iterator, next);

EXTERN_EHLC(Range_Iterator)
