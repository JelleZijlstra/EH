#include "Hash.hpp"

#include "ArgumentError.hpp"
#include "EmptyIterator.hpp"

ehval_p Hash::make(EHInterpreter *parent) {
	return parent->allocate<Hash>(new ehhash_t);
}

EH_INITIALIZER(Hash) {
	REGISTER_METHOD(Hash, toArray);
	REGISTER_METHOD_RENAME(Hash, operator_arrow, "operator->");
	REGISTER_METHOD_RENAME(Hash, operator_arrow_equals, "operator->=");
	REGISTER_METHOD(Hash, has);
	REGISTER_METHOD(Hash, compare);
	REGISTER_METHOD(Hash, delete);
	REGISTER_METHOD(Hash, keys);
	REGISTER_METHOD(Hash, length);
	REGISTER_METHOD(Hash, getIterator);
	REGISTER_CLASS(Hash, Iterator);
}

/*
 * @description Converts the Hash to an Array, preserving all string keys.
 * @argument None
 * @returns Array
 */
EH_METHOD(Hash, toArray) {
	ASSERT_NULL_AND_TYPE(Hash, "Hash.toArray");
	Hash::ehhash_t *hash = obj->get<Hash>();
	ehval_p out = Array::make(ehi->get_parent());
	Array::t *arr = out->get<Array>();
	for(auto &i : hash->members) {
		arr->string_indices[i.first] = i.second;
	}
	return out;
}

/*
 * @description Accesses an element in the hash. If the key does not exist, it returns null.
 * @argument String
 * @returns Value
 */
EH_METHOD(Hash, operator_arrow) {
	ASSERT_RESOURCE(Hash, "Hash.operator->");
	args->assert_type<String>("Hash.operator->", ehi);
	const char *key = args->get<String>();
	if(data->has(key)) {
		return data->get(key);
	} else {
		return Null::make();
	}
}

/*
 * @description Changes the hash to map the key _k_ to the new value _v_
 * @argument Tuple of size two: _k_ and _v_
 * @returns _v_
 */
EH_METHOD(Hash, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, Hash, "Hash.operator->=");
	ehval_p index = args->get<Tuple>()->get(0);
	index->assert_type<String>("Hash.operator->=", ehi);
	ehval_p value = args->get<Tuple>()->get(1);
	Hash::ehhash_t *hash = obj->get<Hash>();
	hash->set(index->get<String>(), value);
	return value;
}

/*
 * @description Checks whether a given key exists in the hash.
 * @argument Key
 * @returns Bool
 */
EH_METHOD(Hash, has) {
	ASSERT_OBJ_TYPE(Hash, "Hash.has");
	args->assert_type<String>("Hash.has", ehi);
	Hash::ehhash_t *hash = obj->get<Hash>();
	return Bool::make(hash->has(args->get<String>()));
}

/*
 * @description Removes a given key from the hash.
 * @argument Key
 * @returns The hash
 */
EH_METHOD(Hash, delete) {
	ASSERT_OBJ_TYPE(Hash, "Hash.has");
	args->assert_type<String>("Hash.has", ehi);
	Hash::ehhash_t *hash = obj->get<Hash>();
	hash->erase(args->get<String>());
	return obj;
}

/*
 * @description Returns an array of the keys of the hash.
 * @argument None
 * @returns Array
 */
EH_METHOD(Hash, keys) {
	ASSERT_NULL_AND_TYPE(Hash, "Hash.toArray");
	ehval_p out = Array::make(ehi->get_parent());
	Array::t *arr = out->get<Array>();
	int index = 0;
	for(auto &i : obj->get<Hash>()->members) {
		arr->int_indices[index++] = String::make(strdup(i.first.c_str()));
	}
	return out;
}

/*
 * @description Compare two hashes
 * @argument Hash to compare to
 * @returns Integer (as specified by Object.compare)
 */
EH_METHOD(Hash, compare) {
	ASSERT_OBJ_TYPE(Hash, "Hash.compare");
	args->assert_type<Hash>("Hash.compare", ehi);
	args = args->data();
	Hash::ehhash_t *lhs = obj->get<Hash>();
	Hash::ehhash_t *rhs = args->get<Hash>();

	Hash::ehhash_t::iterator lhs_it = lhs->begin_iterator();
	Hash::ehhash_t::iterator rhs_it = rhs->begin_iterator();
	Hash::ehhash_t::iterator lhs_end = lhs->end_iterator();
	Hash::ehhash_t::iterator rhs_end = rhs->end_iterator();
	while(true) {
		// check whether we've reached the end
		if(lhs_it == lhs_end) {
			if(rhs_it == rhs_end) {
				break;
			} else {
				return Integer::make(-1);
			}
		} else if(rhs_it == rhs_end) {
			return Integer::make(1);
		}

		// compare keys
		int key_cmp = lhs_it->first.compare(rhs_it->first);
		if(key_cmp != 0) {
			return Integer::make(key_cmp);
		}

		// compare values
		int value_cmp = ehi->compare(lhs_it->second, rhs_it->second, obj);
		if(value_cmp != 0) {
			return Integer::make(value_cmp);
		}

		// continue iteration
		lhs_it++;
		rhs_it++;
	}
	return Integer::make(0);
}

/*
 * @description Determine the size of the hash.
 * @argument None
 * @returns Integer
 */
EH_METHOD(Hash, length) {
	ASSERT_NULL_AND_TYPE(Hash, "Hash.length");
	return Integer::make(static_cast<Integer::type>(obj->get<Hash>()->size()));
}

/*
 * @description Creates an iterator over the hash.
 * @argument None
 * @returns Iterator
 */
EH_METHOD(Hash, getIterator) {
	ASSERT_NULL_AND_TYPE(Hash, "Hash.getIterator");
	ehval_p class_member = obj->get_property("Iterator", obj, ehi);
	return ehi->call_method(class_member, "new", obj, obj);
}

EH_INITIALIZER(Hash_Iterator) {
	REGISTER_METHOD(Hash_Iterator, initialize);
	REGISTER_METHOD(Hash_Iterator, hasNext);
	REGISTER_METHOD(Hash_Iterator, next);
}

ehval_p Hash_Iterator::make(ehval_p hash, EHInterpreter *parent) {
	auto val = new t(hash);
	return parent->allocate<Hash_Iterator>(val);
}

bool Hash_Iterator::t::has_next() {
	return this->current != this->end;
}

ehval_p Hash_Iterator::t::next(EHI *ehi) {
	assert(this->has_next());
	ehval_p tuple[2];
	tuple[0] = String::make(strdup(this->current->first.c_str()));
	tuple[1] = this->current->second;
	this->current++;
	return Tuple::make(2, tuple, ehi->get_parent());
}

/*
 * @description Initializer.
 * @argument Hash to iterate over.
 * @returns N/A
 */
EH_METHOD(Hash_Iterator, initialize) {
	args->assert_type<Hash>("Hash.Iterator.initialize", ehi);
	return Hash_Iterator::make(args, ehi->get_parent());
}

/*
 * @description Checks whether the iterator is already depleted.
 * @argument None
 * @returns Bool
 */
EH_METHOD(Hash_Iterator, hasNext) {
	args->assert_type<Null>("Hash.Iterator.hasNext", ehi);
	ASSERT_RESOURCE(Hash_Iterator, "Hash.Iterator.hasNext");
	return Bool::make(data->has_next());
}

/*
 * @description Returns the next element in the hash.
 * @argument None
 * @returns Tuple of the key and value
 */
EH_METHOD(Hash_Iterator, next) {
	args->assert_type<Null>("Hash.Iterator.next", ehi);
	ASSERT_RESOURCE(Hash_Iterator, "Hash.Iterator.next");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next(ehi);
}

