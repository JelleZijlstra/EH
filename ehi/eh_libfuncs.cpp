/*
 * eh_libfuncs.c
 * Jelle Zijlstra, December 2011
 *
 * Contains definitions of EH library functions
 */
#include "eh.h"
#include "eh_libfuncs.h"
#include "eh.bison.hpp"
#include <cmath>

// TODO: build an argument getter that sets arguments to local variables, checks
// argument number, etcetera

// Note that library functions get their arguments in reverse order

/*
 * printvar
 */

class printvar_t {
private:
	std::map<const void *, bool> seen;
	EHI *ehi;
	
	void retval(ehretval_p in);
	void array(eharray_t *in);
	void object(ehobj_t *in);
	void tuple(ehtuple_t *in);

public:
	printvar_t(ehretval_p in, EHI *_ehi) : seen(), ehi(_ehi) {
		this->retval(in);
	}
};

// helper functions for printvar
void printvar_t::retval(ehretval_p in) {
	switch(in->type()) {
		case null_e:
			printf("null\n");
			break;
		case int_e:
			printf("@int %d\n", in->get_intval());
			break;
		case string_e:
			printf("@string '%s'\n", in->get_stringval());
			break;
		case bool_e:
			if(in->get_boolval())
				printf("@bool true\n");
			else
				printf("@bool false\n");
			break;
		case array_e:
			if(this->seen.count((void *)in->get_arrayval()) == 0) {
				this->seen[(void *)in->get_arrayval()] = true;
				printf("@array [\n");
				this->array(in->get_arrayval());
				printf("]\n");
			} else {
				printf("(recursion)\n");
			}
			break;
		case tuple_e:
		  if(this->seen.count((void *)in->get_tupleval()) == 0) {
		    this->seen[(void *)in->get_tupleval()] = true;
		    printf("@tuple <%d> [\n", in->get_tupleval()->size());
		    this->tuple(in->get_tupleval());
		    printf("]\n");
		  } else {
		    printf("(recursion)");
		  }
		  break;
		case weak_object_e:
		case object_e: {
			ehobj_t *obj;
			if(in->type() == object_e) {
				obj = in->get_objectval();
			} else {
				obj = in->get_weak_objectval();
			}
			if(obj->type_id != func_e) {
				if(this->seen.count((void *)obj) == 0) {
					this->seen[(void *)obj] = true;
					//TODO: print the classname somehow
					const char *name = ehi->repo.get_name(obj->type_id).c_str();
					printf("@object <%s> [\n", name);
					this->object(obj);
					printf("]\n");
				} else {
					printf("(recursion)\n");
				}
			} else {
				ehfunc_t *f = obj->object_data->get_funcval();
				printf("@function <");
				switch(f->type) {
					case user_e:
						printf("user");
						break;
					case lib_e:
						printf("library");
						break;
				}
				printf(">: ");
				for(int i = 0; i < f->argcount; i++) {
					printf("%s", f->args[i].name.c_str());
					if(i + 1 < f->argcount) {
						printf(", ");
					}
				}
				printf("\n");
			}
			break;
		}
		case func_e: {
			ehfunc_t *f = in->get_funcval();
			printf("@function <");
			switch(f->type) {
				case user_e:
					printf("user");
					break;
				case lib_e:
					printf("library");
					break;
			}
			printf(">: ");
			for(int i = 0; i < f->argcount; i++) {
				printf("%s", f->args[i].name.c_str());
				if(i + 1 < f->argcount) {
					printf(", ");
				}
			}
			printf("\n");
			break;
		}
		case type_e:
			printf("@type %s\n", get_typestring(in->get_typeval()));
			break;
		case op_e:
			printf("@op %d\n", in->get_opval()->op);
			break;
		case attribute_e:
			printf("@attribute %d\n", in->get_attributeval());
			break;
		case attributestr_e:
			printf("@attributestr\n");
			break;
		case range_e: {
			ehrange_t *range = in->get_rangeval();
			if(this->seen.count((void *)range) == 0) {
				printf("@range [\n");
				this->retval(range->min);
				this->retval(range->max);
				printf("]\n");
				this->seen[(void *)range] = true;
			} else {
				printf("(recursion)\n");
			}
			break;
		}
		case float_e:
			printf("@float %f\n", in->get_floatval());
			break;
		case resource_e:
			printf("@resource\n");
			break;
		case binding_e:
			// pretend it's just a method
			this->retval(in->get_bindingval()->method);
			break;
		case hash_e:
			printf("@hash [\n");
			HASH_FOR_EACH(in->get_hashval(), i) {
				printf("'%s': ", i->first.c_str());
				this->retval(i->second);
			}
			printf("]\n");
		case base_object_e:
			// Well hidden
			break;
	}
	return;
}
void printvar_t::object(ehobj_t *in) {
	OBJECT_FOR_EACH(in, curr) {
		// ignore $this
		if(curr->first.compare("this") == 0) {
			continue;
		}
		
		printf("%s <", curr->first.c_str());
		switch(curr->second->attribute.visibility) {
			case public_e:
				printf("public,");
				break;
			case private_e:
				printf("private,");
				break;
		}
		switch(curr->second->attribute.isstatic) {
			case static_e:
				printf("static,");
				break;
			case nonstatic_e:
				printf("non-static,");
				break;
		}
		switch(curr->second->attribute.isconst) {
			case const_e:
				printf("constant");
				break;
			case nonconst_e:
				printf("non-constant");
				break;
		}
		printf(">: ");
		this->retval(curr->second->value);
	}
}
void printvar_t::array(eharray_t *in) {
	// iterate over strings
	ARRAY_FOR_EACH_STRING(in, i) {
		printf("'%s' => ", i->first.c_str());
		this->retval(i->second);	
	}
	// and ints
	ARRAY_FOR_EACH_INT(in, i) {
		printf("%d => ", i->first);
		this->retval(i->second);	
	}
}
void printvar_t::tuple(ehtuple_t *in) {
  int size = in->size();
  for(int i = 0; i < size; i++) {
    this->retval(in->get(i));
  }
}

EHLIBFUNC(printvar) {
	// check argument count
	if(nargs != 1) {
		eh_error_argcount_lib("printvar", 1, nargs);
	} else {
		printvar_t printer(args[0], ehi);
	}
	// this function always returns NULL
	return NULL;
}

/*
 * Type checking functions
 */
#define TYPEFUNC(typev) EHLIBFUNC(is_ ## typev) { \
	if(nargs != 1) { \
		eh_error_argcount_lib("is_" #typev, 1, nargs); \
		return NULL; \
	} \
	return ehretval_t::make_bool(args[0]->type() == typev ## _e); \
}

TYPEFUNC(null)
TYPEFUNC(int)
TYPEFUNC(string)
TYPEFUNC(bool)
TYPEFUNC(float)
TYPEFUNC(array)
TYPEFUNC(object)
TYPEFUNC(range)
// get the type of a variable
EHLIBFUNC(get_type) {
	if(nargs != 1) {
		eh_error_argcount_lib("get_type", 1, nargs);
		return NULL;
	}
	return ehretval_t::make_string(strdup(get_typestring(args[0]->type())));
}

/*
 * Including files
 */
EHLIBFUNC(include) {
	if(nargs != 1) {
		eh_error_argcount_lib("include", 1, nargs);
		return NULL;
	}
	if(args[0]->type() != string_e) {
		eh_error_type("argument 0 to include", args[0]->type(), enotice_e);
		ehi->returning = false;
		return NULL;
	}
	// do the work
	FILE *infile = fopen(args[0]->get_stringval(), "r");
	if(!infile) {
		eh_error("Unable to open included file", enotice_e);
		ehi->returning = false;
		return NULL;
	}
	EHParser parser(end_is_end_e, ehi);
	ehretval_p parse_return = parser.parse_file(infile);
	// we're no longer returning
	ehi->returning = false;
	return parse_return;
}

// power
EHLIBFUNC(pow) {
	if(nargs != 2) {
		eh_error_argcount_lib("pow", 2, nargs);
		return NULL;
	}
	if(args[1]->type() == int_e && args[0]->type() == int_e) {
		return ehretval_t::make_int(pow((float) args[1]->get_intval(), (float) args[0]->get_intval()));
	} else if(args[1]->type() == int_e && args[0]->type() == float_e) {
		return ehretval_t::make_float(pow((float) args[1]->get_intval(), args[0]->get_floatval()));
	} else if(args[1]->type() == float_e && args[0]->type() == int_e) {
		return ehretval_t::make_float(pow(args[1]->get_floatval(), (float) args[0]->get_intval()));
	} else if(args[1]->type() == float_e && args[0]->type() == float_e) {
		return ehretval_t::make_float(pow(args[1]->get_floatval(), args[0]->get_floatval()));
	} else {
		eh_error_type("argument 0 to pow", args[1]->type(), enotice_e);
		eh_error_type("argument 1 to pow", args[0]->type(), enotice_e);
		return NULL;
	}
}

EHLIBFUNC(log) {
	if(nargs != 1) {
		eh_error_argcount_lib("log", 1, nargs);
		return NULL;
	}
	ehretval_p arg = ehi->to_float(args[0], context);
	if(arg->type() != float_e) {
		eh_error_type("argument 0 to log", args[0]->type(), enotice_e);
		return NULL;
	}
	return ehretval_t::make_float(log(arg->get_floatval()));
}

EHLIBFUNC(eval) {
	if(nargs != 1) {
		eh_error_argcount_lib("eval", 1, nargs);
		return NULL;
	}
	ehretval_p arg = ehi->to_string(args[0], context);
	if(arg->type() != string_e) {
		eh_error_type("argument 0 to eval", args[0]->type(), enotice_e);
		return NULL;	
	}
	return ehi->parse_string(arg->get_stringval());
}

EHLIBFUNC(getinput) {
	if(nargs != 0) {
		eh_error_argcount_lib("getinput", 0, nargs);
		return NULL;
	}
	// more accurately, getint
	ehretval_p ret = ehretval_t::make_typed(int_e);
	fscanf(stdin, "%d", &(ret->intval));
	return ret;
}

EHLIBFUNC(throw) {
	if(nargs != 1) {
		eh_error_argcount_lib("throw", 1, nargs);
		return NULL;
	}
	throw eh_exception(args[0]);
}
