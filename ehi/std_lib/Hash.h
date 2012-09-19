/*
 * Hash class
 */
#ifndef EH_HASH_H_
#define EH_HASH_H_
#include "std_lib_includes.h"

// hash
class ehhash_t {
private:
	typedef std::map<std::string, ehretval_p> hash;
	hash members;
public:
	typedef hash::const_iterator iterator;
	
	ehhash_t() : members() {}
	
	bool has(const char *key) const {
		return members.count(key);
	}
	void set(const char *key, ehretval_p value) {
		members[key] = value;
	}
	ehretval_p get(const char *key) {
		return members[key];
	}
	void erase(const std::string &key) {
		members.erase(key);
	}
	int size() const {
		return members.size();
	}
	
	iterator begin_iterator() const {
		return members.begin();
	}
	iterator end_iterator() const {
		return members.end();
	}
};
#define HASH_FOR_EACH(obj, varname) for(ehhash_t::iterator varname = (obj)->begin_iterator(), end = (obj)->end_iterator(); varname != end; varname++)

EH_METHOD(Hash, toArray);
EH_METHOD(Hash, operator_arrow);
EH_METHOD(Hash, operator_arrow_equals);
EH_METHOD(Hash, has);
EH_METHOD(Hash, delete);
EH_METHOD(Hash, keys);
EH_METHOD(Hash, length);
EH_METHOD(Hash, getIterator);

EH_INITIALIZER(Hash);

class Hash_Iterator : public LibraryBaseClass {
public:
	Hash_Iterator(ehretval_p _hash) : hash(_hash), current(this->hash->get_hashval()->begin_iterator()), end(this->hash->get_hashval()->end_iterator()) {}
	~Hash_Iterator() {}
	bool has_next();
	ehretval_p next(EHI *ehi);
private:
	ehretval_p hash;
	ehhash_t::iterator current;
	ehhash_t::iterator end;
	Hash_Iterator(const Hash_Iterator&);
	Hash_Iterator operator=(const Hash_Iterator&);
};
EH_METHOD(Hash_Iterator, initialize);
EH_METHOD(Hash_Iterator, hasNext);
EH_METHOD(Hash_Iterator, next);

EH_INITIALIZER(Hash_Iterator);

#endif /* EH_HASH_H_ */
