/*
 * Array
 */

#include "../eh.hpp"

#ifndef EH_ARRAY_H_
#define EH_ARRAY_H_

#include "String.hpp"
#include "Integer.hpp"

EH_CLASS(Array) {
public:
	class t {
	public:
		// typedefs
		typedef std::map<const int, ehval_w> int_map;
		typedef std::map<const std::string, ehval_w> string_map;
		typedef std::pair<const int, ehval_w>& int_pair;
		typedef std::pair<const std::string, ehval_w>& string_pair;
		typedef int_map::iterator int_iterator;
		typedef string_map::iterator string_iterator;

		// properties
		int_map int_indices;
		string_map string_indices;

		// constructor
		t() : int_indices(), string_indices() {}

		// inline methods
		size_t size() const {
			return this->int_indices.size() + this->string_indices.size();
		}

		bool has(ehval_p index) const {
			if(index->is_a<Integer>()) {
				return this->int_indices.count(index->get<Integer>());
			} else if(index->is_a<String>()) {
				return this->string_indices.count(index->get<String>());
			} else {
				return false;
			}
		}

		// methods
		ehval_w &operator[](ehval_p index);
		void insert_retval(ehval_p index, ehval_p value);
		int compare(t *rhs, ehcontext_t context, EHI *ehi);
	};
	typedef t *type;
	type value;

	virtual ~Array() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(auto &i : value->int_indices) {
			out.push_back(i.second);
		}
		for(auto &i : value->string_indices) {
			out.push_back(i.second);
		}
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			std::cout << "@array [" << std::endl;
			for(auto &i : value->string_indices) {
				add_tabs(std::cout, level + 1);
				std::cout << "'" << i.first << "' => ";
				i.second->printvar(set, level + 1, ehi);
			}
			for(auto &i : value->int_indices) {
				add_tabs(std::cout, level + 1);
				std::cout << i.first << " => ";
				i.second->printvar(set, level + 1, ehi);
			}
			add_tabs(std::cout, level);
			std::cout << "]" << std::endl;
		} else {
			std::cout << "(recursion)" << std::endl;
		}
	}

	static ehval_p make(EHInterpreter *parent);

	Array(t *c) : value(c) {}
};

#include "std_lib_includes.hpp"

EH_METHOD(Array, initialize);
EH_METHOD(Array, length);
EH_METHOD(Array, has);
EH_METHOD(Array, operator_arrow);
EH_METHOD(Array, operator_arrow_equals);
EH_METHOD(Array, toArray);
EH_METHOD(Array, toTuple);
EH_METHOD(Array, compare);
EH_METHOD(Array, getIterator);

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
		bool in_ints;
		Array::t::string_iterator string_begin;
		Array::t::string_iterator string_end;
		Array::t::int_iterator int_begin;
		Array::t::int_iterator int_end;
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
EH_METHOD(Array_Iterator, initialize);
EH_METHOD(Array_Iterator, hasNext);
EH_METHOD(Array_Iterator, next);
EH_METHOD(Array_Iterator, peek);

EH_INITIALIZER(Array_Iterator);

#endif /* EH_ARRAY_H_ */
