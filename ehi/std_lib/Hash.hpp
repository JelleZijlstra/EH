/*
 * Hash
 * Provides a mapping from strings to arbitrary values. Despite the name, it
 * currently does not use a hash table, but a C++ std::map.
 */
#include "std_lib_includes.hpp"

#ifndef EH_HASH_H_
#define EH_HASH_H_

EH_CLASS(Hash) {
public:
	class ehhash_t {
	public:
		typedef std::map<std::string, ehval_w> hash;
		hash members;
		typedef hash::const_iterator iterator;

		ehhash_t() : members() {}

		bool has(const char *key) const {
			return members.count(key);
		}
		void set(const char *key, ehval_p val) {
			members[key] = val;
		}
		ehval_p get(const char *key) const {
			return members.at(key);
		}
		void erase(const std::string &key) {
			members.erase(key);
		}
		size_t size() const {
			return members.size();
		}

		iterator begin_iterator() const {
			return members.begin();
		}
		iterator end_iterator() const {
			return members.end();
		}
	};

	typedef ehhash_t *type;
	type value;

	Hash(type c) : value(c) {}

	virtual ~Hash() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(auto &i : value->members) {
			out.push_back(i.second);
		}
		assert(out.size() == value->size());
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		void *ptr = static_cast<void *>(value);
		if(set.count(ptr) == 0) {
			set.insert(ptr);
			std::cout << "@hash [" << std::endl;
			for(auto &i : value->members) {
				add_tabs(std::cout, level + 1);
				std::cout << "'" << i.first << "': ";
				i.second->printvar(set, level + 1, ehi);
			}
			add_tabs(std::cout, level);
			std::cout << "]" << std::endl;
		} else {
			std::cout << "(recursion)" << std::endl;
		}
	}

	static ehval_p make(EHInterpreter *parent);
};

EH_METHOD(Hash, toArray);
EH_METHOD(Hash, operator_arrow);
EH_METHOD(Hash, operator_arrow_equals);
EH_METHOD(Hash, has);
EH_METHOD(Hash, delete);
EH_METHOD(Hash, compare);
EH_METHOD(Hash, keys);
EH_METHOD(Hash, length);
EH_METHOD(Hash, getIterator);

EH_INITIALIZER(Hash);

EH_CHILD_CLASS(Hash, Iterator) {
private:
	class t {
	public:
		t(ehval_p _hash) : hash(_hash), current(this->hash->get<Hash>()->begin_iterator()), end(this->hash->get<Hash>()->end_iterator()) {}
		~t() {}
		bool has_next();
		ehval_p next(EHI *ehi);
		ehval_p hash;
	private:
		Hash::ehhash_t::iterator current;
		Hash::ehhash_t::iterator end;
		t(const t&);
		t operator=(const t&);
	};

public:
	Hash_Iterator(t *c) : value(c) {}

	typedef t *type;
	type value;

	virtual ~Hash_Iterator() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return std::list<ehval_p> { value->hash };
	}

	static ehval_p make(ehval_p hash, EHInterpreter *parent);
};
EH_METHOD(Hash_Iterator, operator_colon);
EH_METHOD(Hash_Iterator, hasNext);
EH_METHOD(Hash_Iterator, next);

EH_INITIALIZER(Hash_Iterator);

#endif /* EH_HASH_H_ */
