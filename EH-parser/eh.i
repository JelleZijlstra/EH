/*
 * eh.i
 * Jelle Zijlstra, December 2011
 *
 * SWIG interface file for ehi
 */
%{
#include "eh.h"
EHI *interpreter;
int EHI::execute_cmd(const char *cmd, ehvar_t **paras) {
	return 0;
}
char *EHI::eh_getline(void) {
	return NULL;
}
EHI::~EHI(void) {
	return;
}

zval *arrtozval(ehvar_t **paras) {
	zval *arr = (zval *)Malloc(sizeof(zval));
	// need to initialize refcount in order to prevent PHP from segfaulting on this.
	arr->refcount__gc = 1;
	arr->is_ref__gc = 0;
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
				}
			}
			currvar = currvar->next;
		}
	}
	return arr;
}
%}
%module(directors="1") ehphp
%feature("director");

// EH string to PHP string
%typemap(directorin) char* {
	zval *str;
	
	MAKE_STD_ZVAL(str);
	ZVAL_STRING(str, rawcmd, 1);
	obj0 = *str;
}
// Typemap from EH array to PHP array
%typemap(directorin) ehvar_t** {
	// It looks like SWIG's directorin support for PHP may be broken. The code on the following line was written after inspection of the generated eh_wrap.cpp code and will not work when the argument names are different from those of EHI::execute_cmd().
	obj1 = *arrtozval(paras);
}

class EHI {
public:
	int eh_interactive(void);
	virtual int execute_cmd(const char *rawcmd, ehvar_t **paras);
	virtual char *eh_getline(void);
	virtual ~EHI();
};
