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

eharray_t *zvaltoeh_array(HashTable *hash);
zval *arrtozval(eharray_t *paras);

zval *ehtozval(ehretval_p in) {
	if(in->type() == array_e) {
		return arrtozval(in->arrayval);
	} else {
		zval *out;
		MAKE_STD_ZVAL(out);
		switch(in->type()) {
			case int_e:
				ZVAL_LONG(out, in->intval);
				break;
			case string_e:
				ZVAL_STRING(out, in->stringval, 0);
				break;
			case bool_e:
				ZVAL_BOOL(out, in->boolval);
				break;
			case array_e:
				break;
			case float_e:
				ZVAL_DOUBLE(out, in->floatval);
				break;
			case null_e:
				ZVAL_NULL(out);
				break;
			case range_e:
			case func_e:
			case object_e:
				// TODO
				eh_error_type("conversion to PHP", in->type(), enotice_e);
				break;
			case accessor_e:
			case type_e:
			case op_e:
			case attribute_e:
			case attributestr_e:
				// these shouldn't even appear as user-visible types
				eh_error_type("conversion to PHP", in->type(), efatal_e);
				break;
		}
		return out;
	}
}

zval *arrtozval(eharray_t *paras) {
	zval *arr;
	MAKE_STD_ZVAL(arr);

	// initiate PHP array
	array_init(arr);
	ARRAY_FOR_EACH_INT(paras, i) {
		add_index_zval(arr, i->first, ehtozval(i->second));
	}
	ARRAY_FOR_EACH_STRING(paras, i) {
		add_assoc_zval(arr, i->first.c_str(), ehtozval(i->second));
	}
	return arr;
}

ehretval_p zvaltoeh(zval *in) {
	ehretval_p ret;
	switch(in->type) {
		case IS_NULL:
			break;
		case IS_BOOL:
			ret->type(bool_e);
			// apparently, a bool is stored as a long
			if(in->value.lval)
				ret->boolval = true;
			else
				ret->boolval = false;
			break;
		case IS_DOUBLE:
			ret->set((float) in->value.dval);
			break;
		case IS_STRING:
			// would be nice to use strndup with in->value.str.len, can't get it though
			ret->set(strdup(in->value.str.val));
			break;
		case IS_ARRAY:
			// initialize array
			ret->set(zvaltoeh_array(in->value.ht));
			break;
		case IS_LONG:
			ret->set((int) in->value.lval);
			break;
		case IS_RESOURCE:
		case IS_OBJECT:
		default:
			fprintf(stderr, "Unsupported PHP type %d\n", in->type);
			break;
	}
	return ret;
}

eharray_t *zvaltoeh_array(HashTable *hash) {
    eharray_t *retval = new eharray_t;
	// variables for our new array
	ehretval_p index, value;
	for(Bucket *curr = hash->pListHead; curr != NULL; curr = curr->pListNext) {
		// apparently the pDataPtr actually points to the zval
		// see Zend/zend_hash.h for definition of the Bucket. No idea
		// what the pData actually points to.
		value = zvaltoeh((zval *)curr->pDataPtr);
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
	*$input = *arrtozval($1);
}
%typemap(directorin) EHParser* {
	ZVAL_NULL($input);
}

// Typemap for returning stuff from execute_cmd
%typemap(directorout) ehretval_p  {
	$result = zvaltoeh($1);
}
%typemap(out) ehretval_t {
	$result = ehtozval(&$1);
}

class EHI {
public:
	int eh_interactive(void);
	ehretval_t parse_file(const char *name);
	ehretval_t parse_string(const char *cmd);

	virtual ehretval_p execute_cmd(const char *name, eharray_t *paras);
	virtual char *eh_getline(EHParser *parser = NULL);
	virtual ~EHI();
};
