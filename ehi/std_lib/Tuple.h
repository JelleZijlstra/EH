/*
 * Tuple class
 */
#ifndef EH_TUPLE_H_
#define EH_TUPLE_H_
#include "std_lib_includes.h"

// EH tuples
class ehtuple_t {
private:
	const int _size;
	ehretval_a content;
public:
	ehtuple_t(int size, ehretval_p *in) : _size(size), content(size) {
		for(int i = 0; i < size; i++) {
			content[i] = in[i];
		}
	}

	int size() const {
		return this->_size;
	}
	ehretval_p get(int i) const {
		assert(i >= 0 && i < _size);
		return this->content[i];
	}
};


EH_METHOD(Tuple, initialize);
EH_METHOD(Tuple, operator_arrow);
EH_METHOD(Tuple, length);
EH_METHOD(Tuple, toTuple);
EH_METHOD(Tuple, getIterator);
EH_METHOD(Tuple, compare);

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

#endif /* EH_TUPLE_H_ */
