/*
 * eh.i
 * Jelle Zijlstra, December 2011
 *
 * SWIG interface file for ehi
 */
%{
#include "eh.h"
#include "eh_interpreter.h"
%}
%module(directors="1") ehphp
%feature("director");

// Typemap from EH array to PHP array
%typemap(in) ehvar_t ** {
	zval *arr;
	ehvar_t *currvar;
	int i;

	// initiate PHP array
	array_init(arr);
	for(i = 0; i < VARTABLE_S; i++) {
		currvar = $input[i];
		while(currvar != NULL) {
			// convert an EH array member to a PHP array member
			if(currvar->indextype == int_e) {
				switch(currvar->type) {
					case int_e:
						add_index_long(arr, currvar->index, currvar->value.intval);
						break;
					case string_e:
						add_index_string(arr, currvar->index, currvar->value.stringval, 0);
						break;
					case bool_e:
						add_index_bool(arr, currvar->index, currvar->value.boolval);
						break;
				}
			}
			else if(currvar->indextype == string_e) {
				switch(currvar->type) {
					case int_e:
						add_assoc_long(arr, currvar->name, currvar->value.intval);
						break;
					case string_e:
						add_assoc_string(arr, currvar->name, currvar->value.stringval, 0);
						break;
					case bool_e:
						add_assoc_bool(arr, currvar->name, currvar->value.boolval);
						break;
				}
			}
			currvar = currvar->next;
		}
	}
	$1 = arr;
}

%include "ehi.h"
