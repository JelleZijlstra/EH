#include "eh.h"
static void free_vartable(ehvar_t **table);

void ehretval_t::free() {
	switch(type) {
		case null_e:
		case int_e:
		case bool_e:
		case float_e:
		case accessor_e:
		case attribute_e:
		case attributestr_e:
		case type_e:
			// nothing to do
			break;
		case range_e:
			delete rangeval;
			break;
		case string_e:
			// string may not be dynamically allocated
			//delete[] stringval;
			break;
		case reference_e:
		case creference_e:
			referenceval->dec_rc();
			break;
		case func_e:
			// need to free something inside the ehfm_t?
			delete funcval;
			break;
		case op_e:
			// ignore this: it'll be handled by the AST destructor, not by the
			// runtime garbage collector
			break;
		case object_e:
			free_vartable(objectval->members);
			delete objectval;
			break;
		case array_e:
			free_vartable(arrayval);
			break;
	}
	// Suicide. Cf. 
	// http://www.parashift.com/c++-faq-lite/freestore-mgmt.html#faq-16.15
	delete this;
}

// free a vartable (as in an array of object)
static void free_vartable(ehvar_t **table) {
	for(int i = 0; i < VARTABLE_S; i++) {
		for(ehvar_t *c = table[i]; c != NULL; ) {
			c->value->dec_rc();
			ehvar_t *oldc = c;
			c = c->next;
			delete oldc;
		}
	}
	delete[] table;
}