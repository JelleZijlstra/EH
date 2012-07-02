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

// get arguments, and error if there are too many or few
// Args should have enough memory malloc'ed to it to house n arguments.
// Return 0 on success, -1 on too few, 1 on too many arguments.
int eh_getargs(ehretval_t *paras, int n, ehretval_t **args, ehcontext_t context, const char *name, EHI *obj) {
	int i = n;
	while(i) {
		if(paras->opval->nparas == 0) {
			eh_error_argcount_lib(name, n, n - i);
			return -1;
		}
		args[--i] = obj->eh_execute(paras->opval->paras[1], context);
		paras = paras->opval->paras[0];
	}
	if(paras->opval->nparas != 0) {
		eh_error_argcount_lib(name, n, n + 1);
		return 1;
	}
	return 0;
}

/*
 * printvar
 */

class printvar_t {
private:
	std::map<const void *, bool> seen;
	
	void retval(ehretval_t *in);
	void array(eharray_t *in);
	void object(ehobj_t *in);

public:
	printvar_t(ehretval_t *in) : seen() {
		this->retval(in);
	}
};

// helper functions for printvar
void printvar_t::retval(ehretval_t *in) {
	switch(EH_TYPE(in)) {
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
		case class_e:
			if(this->seen.count((void *)in->objectval) == 0) {
				this->seen[(void *)in->objectval] = true;
				printf("@class <%s> [\n", in->objectval->classname);
				this->object(in->objectval);
				printf("]\n");
			} else {
				printf("(recursion)\n");
			}
			break;
		case creference_e:
		case reference_e:
			this->retval(in->referenceval);
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
	// this function always returns NULL
	*retval = NULL;

	// check argument count
	ehretval_t *args[1];
	if(eh_getargs(paras, 1, args, context, __FUNCTION__, obj))
		return;
	printvar_t printer(args[0]);
	return;
}

/*
 * Type checking functions
 */
#define TYPEFUNC(typev) EHLIBFUNC(is_ ## typev) { \
	*retval = new ehretval_t(bool_e); \
	ehretval_t *args[1]; \
	if(eh_getargs(paras, 1, args, context, __FUNCTION__, obj)) \
		EHLF_RETFALSE; \
	ehretval_t *value = args[0]; \
	(*retval)->boolval = (value->type == typev ## _e); \
	return; \
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
	ehretval_t *args[2];
	if(eh_getargs(paras, 2, args, context, __FUNCTION__, obj))
		EHLF_RETFALSE;
	if(args[0]->type != string_e) {
		eh_error_type("argument 0 to class_is", args[0]->type, enotice_e);
		EHLF_RETFALSE;
	}
	if(args[1]->type != object_e) {
		eh_error_type("argument 1 to class_is", args[1]->type, enotice_e);
		EHLF_RETFALSE;
	}
	if(!strcmp(args[0]->stringval, args[1]->objectval->classname))
		EHLF_RETTRUE;
	else
		EHLF_RETFALSE;
}
// get the type of a variable
EHLIBFUNC(get_type) {
	ehretval_t *args[1];
	if(eh_getargs(paras, 1, args, context, __FUNCTION__, obj)) {
		EHLF_RETFALSE;
	}
	*retval = new ehretval_t(strdup(get_typestring(args[0]->type)));
}

/*
 * Including files
 */
ehretval_t eh_include_file(FILE *file);
EHLIBFUNC(include) {
	ehretval_t *args[1];
	if(eh_getargs(paras, 1, args, context, __FUNCTION__, obj)) {
		EHLF_RETFALSE;
	}
	if(EH_TYPE(args[0]) != string_e) {
		eh_error_type("argument 0 to include", EH_TYPE(args[0]), enotice_e);
		obj->returning = false;
		EHLF_RETFALSE;
	}
	// do the work
	FILE *infile = fopen(args[0]->stringval, "r");
	if(!infile) {
		eh_error("Unable to open included file", enotice_e);
		obj->returning = false;
		EHLF_RETFALSE;
	}
	EHParser parser(end_is_end_e, obj);
	ehretval_t parse_return = parser.parse_file(infile);
	*retval = (new ehretval_t)->overwrite(&parse_return);
	// we're no longer returning
	obj->returning = false;
	return;
}

// power
EHLIBFUNC(pow) {
	ehretval_t *args[2];
	if(eh_getargs(paras, 2, args, context, __FUNCTION__, obj))
		EHLF_RETFALSE;
	if(args[0]->type == int_e && args[1]->type == int_e) {
		*retval = new ehretval_t((int) pow((float) args[0]->intval, (float) args[1]->intval));
	}
	else if(args[0]->type == int_e && args[1]->type == float_e) {
		*retval = new ehretval_t(pow((float) args[0]->intval, args[1]->floatval));
	}
	else if(args[0]->type == float_e && args[1]->type == int_e) {
		*retval = new ehretval_t(pow(args[0]->floatval, (float) args[1]->intval));
	}
	else if(args[0]->type == float_e && args[1]->type == float_e) {
		*retval = new ehretval_t(pow(args[0]->floatval, args[1]->floatval));
	}
	else {
		eh_error_type("argument 0 to pow", args[0]->type, enotice_e);
		eh_error_type("argument 1 to pow", args[1]->type, enotice_e);
		EHLF_RETFALSE;
	}
}

EHLIBFUNC(log) {
	ehretval_t *args[1];
	if(eh_getargs(paras, 1, args, context, __FUNCTION__, obj))
		EHLF_RETFALSE;
	ehretval_t *arg = eh_xtofloat(args[0]);
	if(EH_TYPE(arg) != float_e) {
		eh_error_type("argument 0 to log", EH_TYPE(args[0]), enotice_e);
		EHLF_RETFALSE;
	}
	*retval = new ehretval_t(log(arg->floatval));
}

EHLIBFUNC(eval) {
	ehretval_t *args[1];
	if(eh_getargs(paras, 1, args, context, __FUNCTION__, obj)) {
		EHLF_RETFALSE;
	}
	ehretval_t *arg = eh_xtostring(args[0]);
	if(EH_TYPE(arg) != string_e) {
		eh_error_type("argument 0 to eval", EH_TYPE(args[0]), enotice_e);
		EHLF_RETFALSE;	
	}
	*retval = new ehretval_t(obj->parse_string(arg->stringval));
}

EHLIBFUNC(getinput) {
	// more accurately, getint
	*retval = new ehretval_t(int_e);
	fscanf(stdin, "%d", &((*retval)->intval));
	return;
}
