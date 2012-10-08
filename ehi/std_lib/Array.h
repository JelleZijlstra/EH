/*
 * Array class
 */
#ifndef EH_ARRAY_H_
#define EH_ARRAY_H_

#include "std_lib_includes.h"

// EH array
class eharray_t {
public:
	// typedefs
	typedef std::map<const int, ehretval_p> int_map;
	typedef std::map<const std::string, ehretval_p> string_map;
	typedef std::pair<const int, ehretval_p>& int_pair;
	typedef std::pair<const std::string, ehretval_p>& string_pair;
	typedef int_map::iterator int_iterator;
	typedef string_map::iterator string_iterator;

	// properties
	int_map int_indices;
	string_map string_indices;
	
	// constructor
	eharray_t() : int_indices(), string_indices() {}
	
	// inline methods
	size_t size() const {
		return this->int_indices.size() + this->string_indices.size();
	}
	
	bool has(ehretval_p index) const {
		switch(index->type()) {
			case int_e:
				return this->int_indices.count(index->get_intval());
			case string_e:
				return this->string_indices.count(index->get_stringval());
			default:
				return false;
		}
	}
	
	// methods
	ehretval_p &operator[](ehretval_p index);
	void insert_retval(ehretval_p index, ehretval_p value);
};
#define ARRAY_FOR_EACH_STRING(array, varname) for(eharray_t::string_iterator varname = (array)->string_indices.begin(), end = (array)->string_indices.end(); varname != end; varname++)
#define ARRAY_FOR_EACH_INT(array, varname) for(eharray_t::int_iterator varname = (array)->int_indices.begin(), end = (array)->int_indices.end(); varname != end; varname++)

EH_METHOD(Array, initialize);
EH_METHOD(Array, length);
EH_METHOD(Array, has);
EH_METHOD(Array, operator_arrow);
EH_METHOD(Array, operator_arrow_equals);
EH_METHOD(Array, toArray);
EH_METHOD(Array, toTuple);
EH_METHOD(Array, getIterator);

EH_INITIALIZER(Array);

class Array_Iterator : public LibraryBaseClass {
public:
	Array_Iterator(ehretval_p array);
	~Array_Iterator() {}
	bool has_next() const;
	ehretval_p next(EHI *ehi);
	ehretval_p peek(EHI *ehi) const;
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
EH_METHOD(Array_Iterator, peek);

EH_INITIALIZER(Array_Iterator);

#endif /* EH_ARRAY_H_ */
