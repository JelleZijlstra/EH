/*
 * eh.i
 * Jelle Zijlstra, December 2011
 *
 * SWIG interface file for ehi
 */
%{
#include "eh.hpp"

ehval_p EHI::execute_cmd(const char *name, Map::t *paras) {
	return nullptr;
}
char *EHI::eh_getline() {
	execute_cmd("man", new Map::t(this));
	return nullptr;
}

ehval_p zvaltoeh_map(HashTable *hash, EHI *ehi);
zval *maptozval(Map::t *paras, EHI *ehi);
zval *hashtozval(Hash::ehhash_t *hash, EHI *ehi);
zval *tupletozval(Tuple::t *tuple, EHI *ehi);

zval *ehtozval(ehval_p in, EHI *ehi) {
	if(in->is_a<Map>()) {
		return maptozval(in->get<Map>(), ehi);
	} else if(in->is_a<Hash>()) {
		return hashtozval(in->get<Hash>(), ehi);
	} else if(in->is_a<Tuple>()) {
		return tupletozval(in->get<Tuple>(), ehi);
	} else {
		zval *out;
		MAKE_STD_ZVAL(out);
		if(in->is_a<Integer>()) {
			ZVAL_LONG(out, in->get<Integer>());
		} else if(in->is_a<String>()) {
			ZVAL_STRING(out, strdup(in->get<String>()), 0);
		} else if(in->is_a<Bool>()) {
			ZVAL_BOOL(out, in->get<Bool>());
		} else if(in->is_a<Float>()) {
			ZVAL_DOUBLE(out, in->get<Float>());
		} else if(in->is_a<Null>()) {
			ZVAL_NULL(out);
		} else {
			throw_TypeError("Unable to convert this type to PHP", in, ehi);
		}
		return out;
	}
}

zval *hashtozval(Hash::ehhash_t *hash, EHI *ehi) {
	zval *arr;
	MAKE_STD_ZVAL(arr);

	array_init(arr);
	for(auto &i : hash->members) {
		add_assoc_zval(arr, i.first.c_str(), ehtozval(i.second, ehi));
	}
	return arr;
}

zval *tupletozval(Tuple::t *tuple, EHI *ehi) {
	zval *arr;
	MAKE_STD_ZVAL(arr);

	array_init(arr);
	for(int i = 0, size = tuple->size(); i < size; i++) {
		add_index_zval(arr, i, ehtozval(tuple->get(i), ehi));
	}
	return arr;
}

zval *maptozval(Map::t *paras, EHI *ehi) {
	zval *arr;
	MAKE_STD_ZVAL(arr);

	// initiate PHP array
	array_init(arr);
	for(auto &i : paras->map) {
		if(i.first->is_a<Integer>()) {
			add_index_zval(arr, i.first->get<Integer>(), ehtozval(i.second, ehi));
		} else if(i.first->is_a<String>()) {
			add_assoc_zval(arr, i.first->get<String>(), ehtozval(i.second, ehi));
		}
	}
	return arr;
}

ehval_p zvaltoeh(zval *in, EHI *ehi) {
	switch(in->type) {
		case IS_NULL:
			return nullptr;
		case IS_BOOL:
			// apparently, a bool is stored as a long
			return Bool::make(in->value.lval);
		case IS_DOUBLE:
			return Float::make(in->value.dval);
		case IS_STRING:
			// would be nice to use strndup with in->value.str.len, can't get it though
			return String::make(strdup(in->value.str.val));
		case IS_ARRAY:
			// initialize array
			return zvaltoeh_map(in->value.ht, ehi);
		case IS_LONG:
			return Integer::make(in->value.lval);
		case IS_RESOURCE:
		case IS_OBJECT:
		default:
			fprintf(stderr, "Unsupported PHP type %d\n", in->type);
			return nullptr;
	}
	return nullptr;
}

ehval_p zvaltoeh_map(HashTable *hash, EHI *ehi) {
    ehval_p ret = Map::make(ehi);
    Map::t *retval = ret->get<Map>();
	// variables for our new array
	ehval_p index, value;
	for(Bucket *curr = hash->pListHead; curr != nullptr; curr = curr->pListNext) {
		// apparently the pDataPtr actually points to the zval
		// see Zend/zend_hash.h for definition of the Bucket. No idea
		// what the pData really points to.
		value = zvaltoeh((zval *)curr->pDataPtr, ehi);
		// determine index type and value
		if(curr->nKeyLength == 0) {
	    	// numeric index
	    	retval->set(Integer::make(curr->h), value);
		} else {
			// string index
			retval->set(String::make(strdup(curr->arKey)), value);
		}
    }
    return ret;
}
%}
%module(directors="1") ehphp
%feature("director");

// EH string to PHP string
%typemap(directorin) char* {
	ZVAL_STRING($input, $1_name, 1);
}
// Typemap from EH array to PHP array
%typemap(directorin) Map::t* {
	*$input = *maptozval($1, this);
}
%typemap(directorin) EHI* {
	ZVAL_NULL($input);
}

// Typemap for returning stuff from execute_cmd
%typemap(directorout) ehval_p  {
	$result = zvaltoeh($1, this);
}
%typemap(out) ehval_p {
	$result = ehtozval($1, arg1);
}

class EHI {
public:
	int eh_interactive();
	ehval_p global_parse_file(const char *name);
	ehval_p global_parse_string(const char *cmd);

	virtual ehval_p execute_cmd(const char *name, Map::t *paras);
	virtual char *eh_getline();
	virtual ~EHI();
};
