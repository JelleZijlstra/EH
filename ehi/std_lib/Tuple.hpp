/*
 * Tuple class
 */
#ifndef EH_TUPLE_H_
#define EH_TUPLE_H_
#include "std_lib_includes.hpp"

EH_CLASS(Tuple) {
public:
	class t {
	protected:
		const int _size;
		ehval_w *content;

	public:
		t(int size, ehval_p *in) : t(size) {
			for(int i = 0; i < size; i++) {
				content[i] = in[i];
			}
		}

		t(int size) : _size(size), content(new ehval_w[size]()) {}

		~t() {
			delete[] content;
		}

		int size() const {
			return this->_size;
		}

		ehval_p get(int i) const {
			assert(is_in_range(i));
			return this->content[i];
		}

		bool is_in_range(int i) const {
			return i >= 0 && i < size();
		}

		friend EH_METHOD(Tuple, initialize);
	};

	typedef t* type;
	type value;

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(int i = 0, len = value->size(); i < len; ++i) {
			out.push_back(value->get(i));
		}
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			const int size = value->size();
			std::cout << "@tuple <" << size << "> [" << std::endl;
			for(int i = 0; i < size; i++) {
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

	static ehval_p make(int size, ehval_p *in, EHInterpreter *parent);

	static ehval_p create(std::initializer_list<ehval_p> members, EHInterpreter *parent) {
		const int size = members.size();
		ehretval_a arr(size);
		int i = 0;
		for(auto &it : members) {
			arr[i] = it;
			i++;
		}
		return make(size, arr, parent);
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
		t(int size) : Tuple::t(size), string_keys() {}

		ehval_p get_string(const std::string &key) const {
			return string_keys.at(key);
		}

		bool has(const std::string &key) const {
			return string_keys.count(key) != 0;
		}

		void set(const std::string &key, ehval_p value) {
			string_keys[key] = value;
		}

		void set(int i, ehval_p value) {
			assert(i >= 0 && i < _size);
			content[i] = value;
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
		for(int i = 0, len = value->size(); i < len; i++) {
			out.push_back(value->get(i));
		}
		for(auto &it : value->string_keys) {
			out.push_back(it.second);
		}
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			const int size = value->size();
			std::cout << "@extended tuple <" << size << "> [" << std::endl;
			for(int i = 0; i < size; i++) {
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
		int position;
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
