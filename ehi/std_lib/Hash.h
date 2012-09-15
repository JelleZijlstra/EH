/*
 * Hash class
 */
#include "std_lib_includes.h"

EH_METHOD(Hash, toArray);
EH_METHOD(Hash, operator_arrow);
EH_METHOD(Hash, operator_arrow_equals);
EH_METHOD(Hash, has);
EH_METHOD(Hash, delete);
EH_METHOD(Hash, keys);
EH_METHOD(Hash, getIterator);

EXTERN_EHLC(Hash)

class Hash_Iterator : public LibraryBaseClass {
public:
	Hash_Iterator(ehretval_p _hash) : hash(_hash), current(this->hash->get_hashval()->begin_iterator()), end(this->hash->get_hashval()->end_iterator()) {}
	~Hash_Iterator() {}
	bool has_next();
	ehretval_p next(EHI *ehi);
private:
	ehretval_p hash;
	ehhash_t::hash_iterator current;
	ehhash_t::hash_iterator end;
	Hash_Iterator(const Hash_Iterator&);
	Hash_Iterator operator=(const Hash_Iterator&);
};
EH_METHOD(Hash_Iterator, initialize);
EH_METHOD(Hash_Iterator, hasNext);
EH_METHOD(Hash_Iterator, next);

EXTERN_EHLC(Hash_Iterator)
