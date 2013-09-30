/*
 * Array
 */

#include "../eh.hpp"

#ifndef EH_ARRAY_H_
#define EH_ARRAY_H_

#include <vector>

#include "String.hpp"
#include "Integer.hpp"

EH_CLASS(Array) {
public:
	class t {
	public:
		// typedefs
		typedef std::vector<ehval_p>::iterator iterator;

		std::vector<ehval_p> v;

		// constructor
		t(int n = 0) : v(n) {}

		// inline methods
		size_t size() const {
			return this->v.size();
		}

		bool has(int index) const {
			return index >= 0 && index < (int) size();
		}

		// methods
		void insert(int index, ehval_p value);
		void append(ehval_p value);
		int compare(t *rhs, ehcontext_t context, EHI *ehi);
	};
	typedef t *type;
	type value;

	virtual ~Array() {
		delete value;
	}

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(auto &i : value->v) {
			out.push_back(i);
		}
		assert(out.size() == value->size());
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			std::cout << "@array [" << std::endl;
			for(auto &i : value->v) {
				add_tabs(std::cout, level + 1);
				i->printvar(set, level + 1, ehi);
			}
			add_tabs(std::cout, level);
			std::cout << "]" << std::endl;
		} else {
			std::cout << "(recursion)" << std::endl;
		}
	}

	static ehval_p make(EHInterpreter *parent, int size = 0);

	Array(t *c) : value(c) {}
};

#include "std_lib_includes.hpp"

EH_METHOD(Array, operator_colon);
EH_METHOD(Array, length);
EH_METHOD(Array, has);
EH_METHOD(Array, operator_arrow);
EH_METHOD(Array, operator_arrow_equals);
EH_METHOD(Array, toArray);
EH_METHOD(Array, toTuple);
EH_METHOD(Array, compare);
EH_METHOD(Array, getIterator);
EH_METHOD(Array, push);

EH_INITIALIZER(Array);

EH_CHILD_CLASS(Array, Iterator) {
private:
	class t {
	public:
		t(ehval_p _array);
		~t() {}
		bool has_next() const;
		ehval_p next(EHI *ehi);
		ehval_p peek(EHI *ehi) const;
		ehval_w array;
	private:
		Array::t::iterator it_begin;
		Array::t::iterator it_end;
		t(const t&);
		t operator=(const t&);
	};

public:
	Array_Iterator(t *val) : value(val) {}

	typedef t *type;
	type value;

	virtual ~Array_Iterator() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return { value->array };
	}

	static ehval_p make(ehval_p array, EHInterpreter *parent);
};
EH_METHOD(Array_Iterator, operator_colon);
EH_METHOD(Array_Iterator, hasNext);
EH_METHOD(Array_Iterator, next);
EH_METHOD(Array_Iterator, peek);

EH_INITIALIZER(Array_Iterator);

#endif /* EH_ARRAY_H_ */
