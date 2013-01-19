/*
 * Tuple class
 */
#ifndef EH_TUPLE_H_
#define EH_TUPLE_H_
#include "std_lib_includes.hpp"

EH_CLASS(Tuple) {
public:
	static ehval_p make(unsigned int size, ehval_p *in, EHInterpreter *parent);

	static ehval_p create(std::initializer_list<ehval_p> members, EHInterpreter *parent) {
		const unsigned int size = static_cast<unsigned int>(members.size());
		ehretval_a arr(size);
		unsigned int i = 0;
		for(auto &it : members) {
			arr[i] = it;
			i++;
		}
		return make(size, arr, parent);
	}

	class t {
	protected:
		const unsigned int _size;
		ehval_w *content;

		t(unsigned int size, ehval_p *in) : t(size) {
			for(unsigned int i = 0; i < size; i++) {
				content[i] = in[i];
			}
		}

		t(unsigned int size) : _size(size), content(new ehval_w[size]()) {}

	public:
		~t() {
			delete[] content;
		}

		unsigned int size() const {
			return this->_size;
		}

		ehval_p get(unsigned int i) const {
			assert(is_in_range(i));
			return this->content[i];
		}

		bool is_in_range(int i) const {
			return i >= 0 && i < static_cast<int>(size());
		}

		friend EH_METHOD(Tuple, initialize);

		friend ehval_p Tuple::make(unsigned int size, ehval_p *in, EHInterpreter *parent);
	};

	typedef t* type;
	type value;

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(unsigned int i = 0, len = value->size(); i < len; ++i) {
			out.push_back(value->get(i));
		}
		assert(out.size() == value->size());
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			const unsigned int size = value->size();
			std::cout << "@tuple <" << size << "> [" << std::endl;
			for(unsigned int i = 0; i < size; i++) {
				add_tabs(std::cout, level + 1);
				value->get(i)->printvar(set, level + 1, ehi);
			}
			add_tabs(std::cout, level);
			std::cout << "]" << std::endl;
		} else {
			std::cout << "(recursion)" << std::endl;
		}
	}

	Tuple(t *val) : value(val) {}

	virtual ~Tuple() {
		delete value;
	}
};

EH_METHOD(Tuple, initialize);
EH_METHOD(Tuple, operator_arrow);
EH_METHOD(Tuple, length);
EH_METHOD(Tuple, toTuple);
EH_METHOD(Tuple, getIterator);
EH_METHOD(Tuple, compare);
EH_METHOD(Tuple, has);

EH_INITIALIZER(Tuple);

EH_CHILD_CLASS(Tuple, WithStringKeys) {
public:
	class t : public Tuple::t {
	private:
		std::unordered_map<std::string, ehval_p> string_keys;

	public:
		t(unsigned int size) : Tuple::t(size), string_keys() {}

		ehval_p get_string(const std::string &key) const {
			return string_keys.at(key);
		}

		bool has(const std::string &key) const {
			return string_keys.count(key) != 0;
		}

		void set(const std::string &key, ehval_p val) {
			string_keys[key] = val;
		}

		void set(unsigned int i, ehval_p val) {
			assert(i >= 0 && i < _size);
			content[i] = val;
		}

		friend class Tuple_WithStringKeys;
	};

	typedef t *type;
	type value;

	Tuple_WithStringKeys(type val) : value(val) {}

	virtual ~Tuple_WithStringKeys() {
		delete value;
	}

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(unsigned int i = 0, len = value->size(); i < len; i++) {
			out.push_back(value->get(i));
		}
		for(auto &it : value->string_keys) {
			out.push_back(it.second);
		}
		assert(out.size() == value->size() + value->string_keys.size());
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			const unsigned int size = value->size();
			std::cout << "@extended tuple <" << size << "> [" << std::endl;
			for(unsigned int i = 0; i < size; i++) {
				add_tabs(std::cout, level + 1);
				value->get(i)->printvar(set, level + 1, ehi);
			}
			for(auto &it : value->string_keys) {
				add_tabs(std::cout, level + 1);
				std::cout << '"' << it.first << "\": ";
				it.second->printvar(set, level + 1, ehi);
			}
			add_tabs(std::cout, level);
			std::cout << "]" << std::endl;
		} else {
			std::cout << "(recursion)" << std::endl;
		}
	}

	static ehval_p make(type in, EHInterpreter *parent);
};

EH_METHOD(Tuple_WithStringKeys, operator_arrow);
EH_METHOD(Tuple_WithStringKeys, has);

EH_INITIALIZER(Tuple_WithStringKeys);

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
		unsigned int position;
		t(const t &);
		t operator=(const t &);

		friend class Tuple_Iterator;
	};

	typedef t *type;
	type value;

	Tuple_Iterator(type val) : value(val) {}

	virtual ~Tuple_Iterator() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return { value->tuple };
	}

	static ehval_p make(ehval_p tuple, EHInterpreter *parent);
};

EH_METHOD(Tuple_Iterator, initialize);
EH_METHOD(Tuple_Iterator, hasNext);
EH_METHOD(Tuple_Iterator, next);

EH_INITIALIZER(Tuple_Iterator);

#endif /* EH_TUPLE_H_ */
