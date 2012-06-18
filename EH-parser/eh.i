/*
 * eh.i
 * Jelle Zijlstra, December 2011
 *
 * SWIG interface file for ehi
 */
%{
#include "eh.h"
ehretval_t *EHI::execute_cmd(const char *name, ehvar_t **paras) {
	return NULL;
}
char *EHI::eh_getline(EHParser *parser) {
	return NULL;
}
EHI::~EHI(void) {
	return;
}

ehvar_t **zvaltoeh_array(HashTable *hash);
zval *arrtozval(ehvar_t **paras);

zval *ehtozval(ehretval_t *in) {
	if(EH_TYPE(in) == array_e) {
		return arrtozval(in->arrayval);
	} else {
		zval *out;
		MAKE_STD_ZVAL(out);
		switch(EH_TYPE(in)) {
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
				eh_error_type("conversion to PHP", EH_TYPE(in), enotice_e);
				break;
			case accessor_e:
			case type_e:
			case reference_e:
			case creference_e:
			case op_e:
			case attribute_e:
			case attributestr_e:
				// these shouldn't even appear as user-visible types
				eh_error_type("conversion to PHP", EH_TYPE(in), efatal_e);
				break;
		}
		return out;
	}
}

zval *arrtozval(ehvar_t **paras) {
	zval *arr;
	MAKE_STD_ZVAL(arr);

	// initiate PHP array
	array_init(arr);
	for(int i = 0; i < VARTABLE_S; i++) {
		for(ehvar_t *currvar = paras[i]; currvar != NULL; currvar = currvar->next) {
			// convert an EH array member to a PHP array member
			if(currvar->indextype == int_e) {
				add_index_zval(arr, currvar->index, ehtozval(currvar->value));
			} else if(currvar->indextype == string_e) {
				add_assoc_zval(arr, currvar->name, ehtozval(currvar->value));
			}
		}
	}
	return arr;
}

ehretval_t *zvaltoeh(zval *in) {
	ehretval_t *ret = NULL;
	switch(in->type) {
		case IS_NULL:
			ret = new ehretval_t(null_e);
			break;
		case IS_BOOL:
			// apparently, a bool is stored as a long
			ret = new ehretval_t(bool_e);
			if(in->value.lval)
				ret->boolval = true;
			else
				ret->boolval = false;
			break;
		case IS_DOUBLE:
			ret = new ehretval_t(in->value.dval);
			break;
		case IS_STRING:
			// would be nice to use strndup with in->value.str.len, can't get it though
			ret = new ehretval_t(strdup(in->value.str.val));
			break;
		case IS_ARRAY:
			// initialize array
			ret = new ehretval_t(array_e);
			ret->arrayval = zvaltoeh_array(in->value.ht);
			break;
		case IS_LONG:
			ret = new ehretval_t((int) in->value.lval);
			break;
		case IS_RESOURCE:
		case IS_OBJECT:
		default:
			fprintf(stderr, "Unsupported PHP type %d\n", in->type);
			break;
	}
	return ret;
}

ehvar_t **zvaltoeh_array(HashTable *hash) {
    ehvar_t **retval = new ehvar_t *[VARTABLE_S]();
	// variables for our new array
	ehretval_t *index, *value;
	for(Bucket *curr = hash->pListHead; curr != NULL; curr = curr->pListNext) {
		// determine index type and value
		if(curr->nKeyLength == 0) {
	    	// numeric index
			index = new ehretval_t(int_e);
			index->intval = curr->h;
		} else {
			// string index
			index = new ehretval_t(strdup(curr->arKey));
		}
		// apparently the pDataPtr actually points to the zval
		// see Zend/zend_hash.h for definition of the Bucket. No idea
		// what the pData actually points to.
		value = zvaltoeh((zval *)curr->pDataPtr);
		array_insert_retval(retval, index, value);
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
%typemap(directorin) ehvar_t** {
	*$input = *arrtozval($1);
}
%typemap(directorin) EHParser* {
	ZVAL_NULL($input);
}

// Typemap for returning stuff from execute_cmd
%typemap(directorout) ehretval_t * {
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

	virtual ehretval_t *execute_cmd(const char *name, ehvar_t **paras);
	virtual char *eh_getline(EHParser *parser = NULL);
	virtual ~EHI();
};
