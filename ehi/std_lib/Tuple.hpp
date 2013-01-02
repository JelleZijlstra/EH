/*
 * Tuple class
 */
#ifndef EH_TUPLE_H_
#define EH_TUPLE_H_
#include "std_lib_includes.hpp"

EH_CLASS(Tuple) {
public:
	class t {
	private:
		const int _size;
		ehretval_a content;

	public:
		t(int size, ehval_p *in) : _size(size), content(size) {
			for(int i = 0; i < size; i++) {
				content[i] = in[i];
			}
		}

		int size() const {
			return this->_size;
		}
		ehval_p get(int i) const {
			assert(i >= 0 && i < _size);
			return this->content[i];
		}

		friend EH_METHOD(Tuple, initialize);
	};

	typedef t* type;
	type value;

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() {
		std::list<ehval_p> out;
		for(int i = 0, len = value->size(); i < len; ++i) {
			out.push_back(value->get(i));
		}
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			const int size = value->size();
			std::cout << "@tuple <" << size << "> [" << std::endl;
			for(int i = 0; i < size; i++) {
				add_tabs(std::cout, level + 1);
				PRINTVAR(value->get(i), level + 1);
			}
			add_tabs(std::cout, level);
			std::cout << "]" << std::endl;
		} else {
			std::cout << "(recursion)" << std::endl;
		}
	}

	Tuple(t *val) : value(val) {}

	~Tuple() {
		delete value;
	}

	static ehval_p make(int size, ehval_p *in, EHInterpreter *parent);
};

EH_METHOD(Tuple, initialize);
EH_METHOD(Tuple, operator_arrow);
EH_METHOD(Tuple, length);
EH_METHOD(Tuple, toTuple);
EH_METHOD(Tuple, getIterator);
EH_METHOD(Tuple, compare);

EH_INITIALIZER(Tuple);

EH_CHILD_CLASS(Tuple, Iterator) {
public:
	class t {
	public:
		t(ehval_p _tuple) : tuple(_tuple), position(0) {}
		~t() {}
		bool has_next();
		ehval_p next();
	private:
		ehval_p tuple;
		int position;
		t(const t &);
		t operator=(const t &);

		friend class Tuple_Iterator;
	};

	typedef t *type;
	type value;

	Tuple_Iterator(type val) : value(val) {}

	~Tuple_Iterator() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() {
		return { value->tuple };
	}

	static ehval_p make(ehval_p tuple, EHInterpreter *parent);
};

EH_METHOD(Tuple_Iterator, initialize);
EH_METHOD(Tuple_Iterator, hasNext);
EH_METHOD(Tuple_Iterator, next);

EH_INITIALIZER(Tuple_Iterator);

#endif /* EH_TUPLE_H_ */
