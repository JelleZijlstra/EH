/*
 * eh.i
 * Jelle Zijlstra, December 2011
 *
 * SWIG interface file for ehi
 */
%{
#include "eh.h"
ehretval_p EHI::execute_cmd(const char *name, eharray_t *paras) {
	return NULL;
}
char *EHI::eh_getline(EHParser *parser) {
	return NULL;
}

eharray_t *zvaltoeh_array(HashTable *hash, EHI *ehi);
zval *arrtozval(eharray_t *paras, EHI *ehi);
zval *hashtozval(ehhash_t *hash, EHI *ehi);
zval *tupletozval(ehtuple_t *tuple, EHI *ehi);

zval *ehtozval(ehretval_p in, EHI *ehi) {
	if(in->type() == array_e) {
		return arrtozval(in->get_arrayval(), ehi);
	} else if(in->type() == hash_e) {
		return hashtozval(in->get_hashval(), ehi);
	} else if(in->type() == tuple_e) {
		return tupletozval(in->get_tupleval(), ehi);
	} else {
		zval *out;
		MAKE_STD_ZVAL(out);
		switch(in->type()) {
			case int_e:
				ZVAL_LONG(out, in->get_intval());
				break;
			case string_e:
				ZVAL_STRING(out, strdup(in->get_stringval()), 0);
				break;
			case bool_e:
				ZVAL_BOOL(out, in->get_boolval());
				break;
			case array_e:
			case hash_e:
			case tuple_e:
				break;
			case float_e:
				ZVAL_DOUBLE(out, in->get_floatval());
				break;
			case null_e:
				ZVAL_NULL(out);
				break;
			case range_e:
			case func_e:
			case binding_e:
			case super_class_e:
			case object_e:
				throw_TypeError("Unable to convert this type to PHP", in->type(), ehi);
				break;
			case type_e:
			case op_e:
			case attribute_e:
			case resource_e:
			case attributestr_e:
				throw_TypeError("Unable to convert this type to PHP", in->type(), ehi);
				break;
		}
		return out;
	}
}

zval *hashtozval(ehhash_t *hash, EHI *ehi) {
	zval *arr;
	MAKE_STD_ZVAL(arr);
	
	array_init(arr);
	HASH_FOR_EACH(hash, i) {
		add_assoc_zval(arr, i->first.c_str(), ehtozval(i->second, ehi));
	}
	return arr;
}

zval *tupletozval(ehtuple_t *tuple, EHI *ehi) {
	zval *arr;
	MAKE_STD_ZVAL(arr);
	
	array_init(arr);
	for(int i = 0, size = tuple->size(); i < size; i++) {
		add_index_zval(arr, i, ehtozval(tuple->get(i), ehi));
	}
	return arr;
}

zval *arrtozval(eharray_t *paras, EHI *ehi) {
	zval *arr;
	MAKE_STD_ZVAL(arr);

	// initiate PHP array
	array_init(arr);
	ARRAY_FOR_EACH_INT(paras, i) {
		add_index_zval(arr, i->first, ehtozval(i->second, ehi));
	}
	ARRAY_FOR_EACH_STRING(paras, i) {
		add_assoc_zval(arr, i->first.c_str(), ehtozval(i->second, ehi));
	}
	return arr;
}

ehretval_p zvaltoeh(zval *in, EHI *ehi) {
	switch(in->type) {
		case IS_NULL:
			return NULL;
		case IS_BOOL:
			// apparently, a bool is stored as a long
			return ehretval_t::make_bool(in->value.lval);
		case IS_DOUBLE:
			return ehretval_t::make_float(in->value.dval);
		case IS_STRING:
			// would be nice to use strndup with in->value.str.len, can't get it though
			return ehretval_t::make_string(strdup(in->value.str.val));
		case IS_ARRAY:
			// initialize array
			return ehi->make_array(zvaltoeh_array(in->value.ht, ehi));
		case IS_LONG:
			return ehretval_t::make_int(in->value.lval);
		case IS_RESOURCE:
		case IS_OBJECT:
		default:
			fprintf(stderr, "Unsupported PHP type %d\n", in->type);
			return NULL;
	}
	return NULL;
}

eharray_t *zvaltoeh_array(HashTable *hash, EHI *ehi) {
    eharray_t *retval = new eharray_t;
	// variables for our new array
	ehretval_p index, value;
	for(Bucket *curr = hash->pListHead; curr != NULL; curr = curr->pListNext) {
		// apparently the pDataPtr actually points to the zval
		// see Zend/zend_hash.h for definition of the Bucket. No idea
		// what the pData actually points to.
		value = zvaltoeh((zval *)curr->pDataPtr, ehi);
		// determine index type and value
		if(curr->nKeyLength == 0) {
	    	// numeric index
	    	retval->int_indices[curr->h] = value;
		} else {
			// string index
			retval->string_indices[curr->arKey] = value;
		}
    }
    return retval;
}
%}
%module(directors="1") ehphp
%feature("director");

// EH string to PHP string
%typemap(directorin) char* {
	ZVAL_STRING($input, $1_name, 1);
}
// Typemap from EH array to PHP array
%typemap(directorin) eharray_t* {
	*$input = *arrtozval($1, this);
}
%typemap(directorin) EHParser* {
	ZVAL_NULL($input);
}

// Typemap for returning stuff from execute_cmd
%typemap(directorout) ehretval_p  {
	$result = zvaltoeh($1, this);
}
%typemap(out) ehretval_t {
	$result = ehtozval(&$1, this);
}

class EHI {
public:
	int eh_interactive(void);
	ehretval_p global_parse_file(const char *name);
	ehretval_p global_parse_string(const char *cmd);

	virtual ehretval_p execute_cmd(const char *name, eharray_t *paras);
	virtual char *eh_getline(EHParser *parser = NULL);
	virtual ~EHI();
};
