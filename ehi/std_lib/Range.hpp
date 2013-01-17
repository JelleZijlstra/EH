/*
 * Range class
 */
#include "std_lib_includes.hpp"
#ifndef EH_RANGE_H_
#define EH_RANGE_H_

#include "Integer.hpp"

EH_CLASS(Range) {
public:
	class t {
	public:
		ehval_w min;
		ehval_w max;

		t(ehval_p _min, ehval_p _max) : min(_min), max(_max) {
			assert(min->equal_type(max));
		}
	};

	typedef t *type;
	type value;

	Range(t *c) : value(c) {}

	~Range() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return { value->min, value->max };
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			std::cout << "@range [" << std::endl;
			add_tabs(std::cout, level + 1);
			value->min->printvar(set, level + 1, ehi);
			add_tabs(std::cout, level + 1);
			value->max->printvar(set, level + 1, ehi);
			add_tabs(std::cout, level);
			std::cout << "]" << std::endl;
		} else {
			std::cout << "(recursion)" << std::endl;
		}
	}

	static ehval_p make(ehval_p min, ehval_p max, EHInterpreter *parent);
};

EH_METHOD(Range, initialize);
EH_METHOD(Range, operator_arrow);
EH_METHOD(Range, min);
EH_METHOD(Range, max);
EH_METHOD(Range, toString);
EH_METHOD(Range, toArray);
EH_METHOD(Range, toRange);
EH_METHOD(Range, compare);
EH_METHOD(Range, getIterator);

EH_INITIALIZER(Range);

EH_CHILD_CLASS(Range, Iterator) {
public:
	class t {
	public:
		t(ehval_p _range) : range(_range), current(this->range->get<Range>()->min) {}

		bool has_next(EHI *ehi);
		ehval_p next(EHI *ehi);
		ehval_p range;
		ehval_p current;

	private:
		t(const t&);
		t operator=(const t&);
	};

	typedef t *type;
	type value;

	Range_Iterator(type val) : value(val) {}

	~Range_Iterator() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return { value->range, value->current };
	}

	static ehval_p make(ehval_p range, EHInterpreter *parent);
};
EH_METHOD(Range_Iterator, initialize);
EH_METHOD(Range_Iterator, hasNext);
EH_METHOD(Range_Iterator, next);

EH_INITIALIZER(Range_Iterator);

#endif /* EH_RANGE_H_ */
