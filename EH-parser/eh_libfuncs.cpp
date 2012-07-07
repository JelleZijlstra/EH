/*
 * eh_libfuncs.c
 * Jelle Zijlstra, December 2011
 *
 * Contains definitions of EH library functions
 */
#include "eh.h"
#include "eh_libfuncs.h"
#include "eh_error.h"
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
	
	void retval(ehretval_p in);
	void array(eharray_t *in);
	void object(ehobj_t *in);

public:
	printvar_t(ehretval_p in) : seen() {
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
			printf("@int %d\n", in->intval);
			break;
		case string_e:
			printf("@string '%s'\n", in->stringval);
			break;
		case bool_e:
			if(in->boolval)
				printf("@bool true\n");
			else
				printf("@bool false\n");
			break;
		case array_e:
			if(this->seen.count((void *)in->arrayval) == 0) {
				this->seen[(void *)in->arrayval] = true;
				printf("@array [\n");
				this->array(in->arrayval);
				printf("]\n");
			} else {
				printf("(recursion)\n");
			}
			break;
		case weak_object_e:
		case object_e:
			if(this->seen.count((void *)in->objectval) == 0) {
				this->seen[(void *)in->objectval] = true;
				printf("@object <%s> [\n", in->objectval->classname);
				this->object(in->objectval);
				printf("]\n");
			} else {
				printf("(recursion)\n");
			}
			break;
		case func_e:
			printf("@function <");
			switch(in->funcval->function->type) {
				case user_e:
					printf("user");
					break;
				case lib_e:
					printf("library");
					break;
				case libmethod_e:
					printf("library method");
					break;
			}
			printf(">: ");
			for(int i = 0; i < in->funcval->function->argcount; i++) {
				printf("%s", in->funcval->function->args[i].name);
				if(i + 1 < in->funcval->function->argcount)
					printf(", ");
			}
			printf("\n");
			break;
		case accessor_e:
			printf("@accesor %d\n", in->accessorval);
			break;
		case type_e:
			printf("@type %s\n", get_typestring(in->typeval));
			break;
		case op_e:
			printf("@op %d\n", in->opval->op);
			break;
		case attribute_e:
			printf("@attribute %d\n", in->attributeval);
			break;
		case attributestr_e:
			printf("@attributestr\n");
			break;
		case range_e:
			printf("@range %d..%d\n", in->rangeval->min, in->rangeval->max);
			break;
		case float_e:
			printf("@float %f\n", in->floatval);
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

EHLIBFUNC(printvar) {
	// check argument count
	if(nargs != 1) {
		eh_error_argcount_lib("printvar", 1, nargs);
	} else {
		printvar_t printer(args[0]);
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
	return ehretval_t::make(args[0]->type() == typev ## _e); \
}

TYPEFUNC(null)
TYPEFUNC(int)
TYPEFUNC(string)
TYPEFUNC(bool)
TYPEFUNC(float)
TYPEFUNC(array)
TYPEFUNC(object)
TYPEFUNC(range)
// check whether a variable is a member of a specified class
EHLIBFUNC(class_is) {
	if(nargs != 2) {
		eh_error_argcount_lib("class_is", 2, nargs);
		return NULL;
	}
	// they are in reverse order
	if(args[1]->type() != string_e) {
		eh_error_type("argument 0 to class_is", args[1]->type(), enotice_e);
		return NULL;
	}
	if(args[0]->type() != object_e) {
		eh_error_type("argument 1 to class_is", args[0]->type(), enotice_e);
		return NULL;
	}
	return ehretval_t::make(strcmp(args[1]->stringval, args[0]->objectval->classname) == 0);
}
// get the type of a variable
EHLIBFUNC(get_type) {
	if(nargs != 1) {
		eh_error_argcount_lib("get_type", 1, nargs);
		return NULL;
	}
	return ehretval_t::make(strdup(get_typestring(args[0]->type())));
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
		obj->returning = false;
		return NULL;
	}
	// do the work
	FILE *infile = fopen(args[0]->stringval, "r");
	if(!infile) {
		eh_error("Unable to open included file", enotice_e);
		obj->returning = false;
		return NULL;
	}
	EHParser parser(end_is_end_e, obj);
	ehretval_p parse_return = parser.parse_file(infile);
	// we're no longer returning
	obj->returning = false;
	return parse_return;
}

// power
EHLIBFUNC(pow) {
	if(nargs != 2) {
		eh_error_argcount_lib("pow", 2, nargs);
		return NULL;
	}
	if(args[1]->type() == int_e && args[0]->type() == int_e) {
		return ehretval_t::make((int) pow((float) args[1]->intval, (float) args[0]->intval));
	} else if(args[1]->type() == int_e && args[0]->type() == float_e) {
		return ehretval_t::make(pow((float) args[1]->intval, args[0]->floatval));
	} else if(args[1]->type() == float_e && args[0]->type() == int_e) {
		return ehretval_t::make(pow(args[1]->floatval, (float) args[0]->intval));
	} else if(args[1]->type() == float_e && args[0]->type() == float_e) {
		return ehretval_t::make(pow(args[1]->floatval, args[0]->floatval));
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
	ehretval_p arg = eh_xtofloat(args[0]);
	if(arg->type() != float_e) {
		eh_error_type("argument 0 to log", args[0]->type(), enotice_e);
		return NULL;
	}
	return ehretval_t::make(log(arg->floatval));
}

EHLIBFUNC(eval) {
	if(nargs != 1) {
		eh_error_argcount_lib("eval", 1, nargs);
		return NULL;
	}
	ehretval_p arg = eh_xtostring(args[0]);
	if(arg->type() != string_e) {
		eh_error_type("argument 0 to eval", args[0]->type(), enotice_e);
		return NULL;	
	}
	return obj->parse_string(arg->stringval);
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
