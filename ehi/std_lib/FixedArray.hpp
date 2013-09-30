/*
 * FixedArray library class
 */
#include "std_lib_includes.hpp"

EH_CLASS(FixedArray) {
public:
	class t {
	public:
		size_t size() const {
			return size_;
		}
		ehval_p get(size_t index) const {
			assert(index < size_);
			return content[index];
		}
		void set(size_t index, ehval_p val) {
			assert(index < size_);
			content[index] = val;
		}
		t(size_t size) : size_(size), content(new ehval_w[size]()) {}
		~t() {
			delete[] content;
		}
	private:
		const size_t size_;
		ehval_w *content;
		t(const t&);
		t operator=(const t&);
	};

	typedef t *type;
	type value;

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(unsigned int i = 0, len = value->size(); i < len; i++) {
			out.push_back(value->get(i));
		}
		assert(out.size() == value->size());
		return out;
	}

	FixedArray(type val) : value(val) {}

	virtual ~FixedArray() {
		delete value;
	}

	static ehval_p make(size_t size, EHInterpreter *parent) {
		return parent->allocate<FixedArray>(new t(size));
	}
};
EH_METHOD(FixedArray, operator_colon);
EH_METHOD(FixedArray, operator_arrow);
EH_METHOD(FixedArray, operator_arrow_equals);
EH_METHOD(FixedArray, size);

EH_INITIALIZER(FixedArray);
