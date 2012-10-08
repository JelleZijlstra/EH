/*
 * FixedArray library class
 */
#include "std_lib_includes.h"

class FixedArray : public LibraryBaseClass {
public:
	size_t size() const {
		return size_;
	}
	ehretval_p get(size_t index) const {
		assert(index >= 0 && index < size_);
		return content[index];
	}
	void set(size_t index, ehretval_p value) {
		assert(index >= 0 && index < size_);
		content[index] = value;		
	}
	FixedArray(size_t size) : size_(size), content(new ehretval_p[size]()) {}
	~FixedArray() {
		delete[] content;
	}
private:
	const size_t size_;
	ehretval_p *content;
	FixedArray(const FixedArray&);
	FixedArray operator=(const FixedArray&);
};
EH_METHOD(FixedArray, initialize);
EH_METHOD(FixedArray, operator_arrow);
EH_METHOD(FixedArray, operator_arrow_equals);
EH_METHOD(FixedArray, size);

EH_INITIALIZER(FixedArray);
