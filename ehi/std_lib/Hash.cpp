#include "Hash.h"

START_EHLC(Hash)
EHLC_ENTRY(Hash, toArray)
EHLC_ENTRY_RENAME(Hash, operator_arrow, "operator->")
EHLC_ENTRY_RENAME(Hash, operator_arrow_equals, "operator->=")
EHLC_ENTRY(Hash, has)
EHLC_ENTRY(Hash, delete)
EHLC_ENTRY(Hash, keys)
END_EHLC()

EH_METHOD(Hash, toArray) {
	ASSERT_NULL_AND_TYPE(hash_e, "Hash.toArray");
	ehhash_t *hash = obj->get_hashval();
	eharray_t *arr = new eharray_t();
	ehretval_p out = ehi->make_array(arr);
	HASH_FOR_EACH(hash, i) {
		arr->string_indices[i->first.c_str()] = i->second;
	}
	return out;
}

EH_METHOD(Hash, operator_arrow) {
	ASSERT_OBJ_TYPE(hash_e, "Hash.operator->");
	ASSERT_TYPE(args, string_e, "Hash.operator->");
	ehhash_t *hash = obj->get_hashval();
	return hash->get(args->get_stringval());
}

EH_METHOD(Hash, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, hash_e, "Hash.operator->=");
	ehretval_p index = args->get_tupleval()->get(0);
	ASSERT_TYPE(index, string_e, "Hash.operator->=");
	ehretval_p value = args->get_tupleval()->get(1);
	ehhash_t *hash = obj->get_hashval();
	hash->set(index->get_stringval(), value);
	return value;
}

EH_METHOD(Hash, has) {
	ASSERT_OBJ_TYPE(hash_e, "Hash.has");
	ASSERT_TYPE(args, string_e, "Hash.has");
	ehhash_t *hash = obj->get_hashval();
	return ehretval_t::make_bool(hash->has(args->get_stringval()));
}

EH_METHOD(Hash, delete) {
	ASSERT_OBJ_TYPE(hash_e, "Hash.has");
	ASSERT_TYPE(args, string_e, "Hash.has");
	ehhash_t *hash = obj->get_hashval();
	hash->erase(args->get_stringval());
	return obj;
}

EH_METHOD(Hash, keys) {
	ASSERT_NULL_AND_TYPE(hash_e, "Hash.toArray");
	eharray_t *arr = new eharray_t();
	int index = 0;
	HASH_FOR_EACH(obj->get_hashval(), i) {
		std::string name = i->first;
		arr->int_indices[index] = ehretval_t::make_string(strdup(name.c_str()));
		index++;
	}
	return ehi->make_array(arr);	
}
