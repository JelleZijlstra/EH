/*
 * eh.i
 * Jelle Zijlstra, December 2011
 *
 * SWIG interface file for ehi
 */
%{
#include "eh.h"
EHI *interpreter;
ehretval_t EHI::execute_cmd(const char *name, ehvar_t **paras) {
	ehretval_t ret;
	ret.type = null_e;
	return ret;
}
char *EHI::eh_getline(void) {
	return NULL;
}
EHI::~EHI(void) {
	return;
}

ehvar_t **zvaltoeh_array(HashTable *hash);

zval *arrtozval(ehvar_t **paras) {
	zval *arr;
	MAKE_STD_ZVAL(arr);
	ehvar_t *currvar;
	int i;

	// initiate PHP array
	array_init(arr);
	for(i = 0; i < VARTABLE_S; i++) {
		currvar = paras[i];
		while(currvar != NULL) {
			// convert an EH array member to a PHP array member
			if(currvar->indextype == int_e) {
				switch(currvar->value.type) {
					case int_e:
						add_index_long(arr,
							currvar->index, currvar->value.intval);
						break;
					case string_e:
						add_index_string(arr,
							currvar->index, currvar->value.stringval, 0);
						break;
					case bool_e:
						add_index_bool(arr,
							currvar->index, currvar->value.boolval);
						break;
					case array_e:
						add_index_zval(arr,
							currvar->index,
							arrtozval(currvar->value.arrayval));
						break;
					case float_e:
						add_index_double(arr,
							currvar->index, currvar->value.floatval);
						break;
					case null_e:
						add_index_null(arr, currvar->index);
						break;
					default:
						eh_error_type("conversion to PHP", currvar->value.type, enotice_e);
						break;
				}
			}
			else if(currvar->indextype == string_e) {
				switch(currvar->value.type) {
					case int_e:
						add_assoc_long(arr,
							currvar->name, currvar->value.intval);
						break;
					case string_e:
						add_assoc_string(arr,
							currvar->name, currvar->value.stringval, 0);
						break;
					case bool_e:
						add_assoc_bool(arr,
							currvar->name, currvar->value.boolval);
						break;
					case array_e:
						add_assoc_zval(arr,
							currvar->name,
							arrtozval(currvar->value.arrayval));
						break;
					case float_e:
						add_assoc_double(arr,
							currvar->name, currvar->value.floatval);
						break;
					case null_e:
						add_assoc_null(arr, currvar->name);
						break;
					default:
						eh_error_type("conversion to PHP", currvar->value.type, enotice_e);
						break;
				}
			}
			currvar = currvar->next;
		}
	}
	return arr;
}

ehretval_t zvaltoeh(zval *in) {
	ehretval_t ret;
	ret.type = null_e;
	switch(in->type) {
		case IS_NULL:
			// don't do anything
			break;
		case IS_BOOL:
			// apparently, a bool is stored as a long
			ret.type = bool_e;
			if(in->value.lval)
				ret.boolval = true;
			else
				ret.boolval = false;
			break;
		case IS_DOUBLE:
			ret.type = float_e;
			ret.floatval = in->value.dval;
			break;
		case IS_STRING:
			ret.type = string_e;
			// would be nice to use strndup with in->value.str.len, can't get it though
			ret.stringval = strdup(in->value.str.val);
			break;
		case IS_ARRAY:
			// initialize array
			ret.type = array_e;
			ret.arrayval = zvaltoeh_array(in->value.ht);
			break;
		case IS_LONG:
			ret.type = int_e;
			ret.intval = in->value.lval;
			break;
		case IS_RESOURCE:
		case IS_OBJECT:
		default:
			fprintf(stderr, "Unsupported PHP type\n");
			break;
	}
	return ret;
}

ehvar_t **zvaltoeh_array(HashTable *hash) {
    ehvar_t** retval = (ehvar_t **) Calloc(VARTABLE_S, sizeof(ehvar_t *));
	// variables for our new array
	ehretval_t index, value;
	for(Bucket *curr = hash->pListHead; curr != NULL; curr = curr->pListNext) {
		// determine index type and value
		if(curr->nKeyLength == 0) {
	    	// numeric index
			index.type = int_e;
			index.intval = curr->h;
		} else {
			// string index
			index.type = string_e;
			index.stringval = strdup(curr->arKey);
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
	zval *str;

	MAKE_STD_ZVAL(str);
	ZVAL_STRING(str, name, 1);
	obj0 = *str;
}
// Typemap from EH array to PHP array
%typemap(directorin) ehvar_t** {
	// It looks like SWIG's directorin support for PHP may be broken. The code on the following line was written after inspection of the generated eh_wrap.cpp code and will not work when the argument names are different from those of EHI::execute_cmd().
	obj1 = *arrtozval(paras);
}

// Typemap for returning stuff from execute_cmd
%typemap(directorout) ehretval_t {
	// provisional, we'll want to work on making it actually do something useful with what PHP returns
	c_result = zvaltoeh(result);
}

class EHI {
public:
	int eh_interactive(void);
	virtual ehretval_t execute_cmd(const char *name, ehvar_t **paras);
	virtual void exec_file(const char *name);
	virtual char *eh_getline(void);
	virtual ~EHI();
};
