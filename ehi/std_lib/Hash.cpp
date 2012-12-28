#include "Hash.hpp"

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

EH_METHOD(Hash, toArray) {
	ASSERT_NULL_AND_TYPE(hash_e, "Hash.toArray");
	ehhash_t *hash = obj->get_hashval();
	eharray_t *arr = new eharray_t();
	ehretval_p out = ehi->get_parent()->make_array(arr);
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
	return ehi->get_parent()->make_array(arr);
}

/*
 * @description Compare two hashes
 * @argument Hash to compare to
 * @returns Integer (as specified by Object.compare)
 */
EH_METHOD(Hash, compare) {
	ASSERT_OBJ_TYPE(hash_e, "Hash.compare");
	ASSERT_TYPE(args, hash_e, "Hash.compare");
	args = ehretval_t::self_or_data(args);
	ehhash_t *lhs = obj->get_hashval();
	ehhash_t *rhs = args->get_hashval();

	ehhash_t::iterator lhs_it = lhs->begin_iterator();
	ehhash_t::iterator rhs_it = rhs->begin_iterator();
	ehhash_t::iterator lhs_end = lhs->end_iterator();
	ehhash_t::iterator rhs_end = rhs->end_iterator();
	while(true) {
		// check whether we've reached the end
		if(lhs_it == lhs_end) {
			if(rhs_it == rhs_end) {
				break;
			} else {
				return ehretval_t::make_int(-1);
			}
		} else if(rhs_it == rhs_end) {
			return ehretval_t::make_int(1);
		}

		// compare keys
		int key_cmp = lhs_it->first.compare(rhs_it->first);
		if(key_cmp != 0) {
			return ehretval_t::make_int(key_cmp);
		}

		// compare values
		int value_cmp = ehi->compare(lhs_it->second, rhs_it->second, obj);
		if(value_cmp != 0) {
			return ehretval_t::make_int(value_cmp);
		}

		// continue iteration
		lhs_it++;
		rhs_it++;
	}
	return ehretval_t::make_int(0);
}

EH_METHOD(Hash, length) {
	ASSERT_NULL_AND_TYPE(hash_e, "Hash.length");
	return ehretval_t::make_int(obj->get_hashval()->size());
}

EH_METHOD(Hash, getIterator) {
	ASSERT_NULL_AND_TYPE(hash_e, "Hash.getIterator");
	ehretval_p class_member = ehi->get_property(obj, "Iterator", obj);
	return ehi->call_method(class_member, "new", obj, obj);
}

EH_INITIALIZER(Hash_Iterator) {
	REGISTER_METHOD(Hash_Iterator, initialize);
	REGISTER_METHOD(Hash_Iterator, hasNext);
	REGISTER_METHOD(Hash_Iterator, next);
}

bool Hash_Iterator::has_next() {
	return this->current != this->end;
}
ehretval_p Hash_Iterator::next(EHI *ehi) {
	assert(this->has_next());
	ehretval_p tuple[2];
	tuple[0] = ehretval_t::make_string(strdup(this->current->first.c_str()));
	tuple[1] = this->current->second;
	this->current++;
	return ehi->get_parent()->make_tuple(new ehtuple_t(2, tuple));
}

EH_METHOD(Hash_Iterator, initialize) {
	ASSERT_TYPE(args, hash_e, "Hash.Iterator.initialize");
	Hash_Iterator *data = new Hash_Iterator(args);
	return ehretval_t::make_resource(obj->get_full_type(), data);
}
EH_METHOD(Hash_Iterator, hasNext) {
	ASSERT_TYPE(args, null_e, "Hash.Iterator.hasNext");
	ASSERT_RESOURCE(Hash_Iterator, "Hash.Iterator.hasNext");
	return ehretval_t::make_bool(data->has_next());
}
EH_METHOD(Hash_Iterator, next) {
	ASSERT_TYPE(args, null_e, "Hash.Iterator.next");
	ASSERT_RESOURCE(Hash_Iterator, "Hash.Iterator.next");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return data->next(ehi);
}

