/*
 * eh_libfuncs.c
 * Jelle Zijlstra, December 2011
 *
 * Contains definitions of EH library functions
 */
#include "eh.h"
#include "eh_libfuncs.h"
#include "eh.bison.hpp"

void printvar_retval(ehretval_t in);
static void printvar_array(ehvar_t **in);
static void printvar_object(ehclassmember_t **in);

// get arguments, and error if there are too many or few
// Args should have enough memory malloc'ed to it to house n arguments.
// Return 0 on success, -1 on too few, 1 on too many arguments.
static int eh_getargs(ehretval_t *paras, int n, ehretval_t *args, ehcontext_t context, const char *name) {
	int i = n;
	while(i) {
		if(paras->opval->nparas == 0) {
			eh_error_argcount_lib(name, n, n - i);
			return -1;
		}
		args[--i] = eh_execute(paras->opval->paras[1], context);
		paras = paras->opval->paras[0];
	}
	if(paras->opval->nparas != 0) {
		eh_error_argcount_lib(name, n, n + 1);
		return 1;
	}
	return 0;
}

EHLIBFUNC(getinput) {
	// more accurately, getint
	retval->type = int_e;
	fscanf(stdin, "%d", &retval->intval);
	return;
}

EHLIBFUNC(printvar) {
	// this function always returns NULL
	retval->type = null_e;

	// check argument count
	ehretval_t *args = (ehretval_t *) Malloc(1 * sizeof(ehretval_t));
	if(eh_getargs(paras, 1, args, context, __FUNCTION__))
		return;
	printvar_retval(args[0]);
	return;
}
// helper functions for printvar
void printvar_retval(ehretval_t in) {
	int i;
	switch(in.type) {
		case null_e:
			printf("null\n");
			break;
		case int_e:
			printf("@int %d\n", in.intval);
			break;
		case string_e:
			printf("@string '%s'\n", in.stringval);
			break;
		case array_e:
			printf("@array [\n");
			printvar_array(in.arrayval);
			printf("]\n");
			break;
		case bool_e:
			if(in.boolval)
				printf("@bool true\n");
			else
				printf("@bool false\n");
			break;
		case object_e:
			printf("@object <%s> [\n", in.objectval->classname);
			printvar_object(in.objectval->members);
			printf("]\n");
			break;
		case creference_e:
		case reference_e:
			printvar_retval(*in.referenceval);
			break;
		case func_e:
			printf("@function <");
			switch(in.funcval->type) {
				case user_e:
					printf("user");
					break;
				case lib_e:
					printf("library");
					break;
			}
			printf(">: ");
			for(i = 0; i < in.funcval->argcount; i++) {
				printf("%s", in.funcval->args[i].name);
				if(i + 1 < in.funcval->argcount)
					printf(", ");
			}
			printf("\n");
			break;
		case accessor_e:
			printf("@accesor %d\n", in.accessorval);
			break;
		case type_e:
			printf("@type %s\n", get_typestring(in.typeval));
			break;
		case magicvar_e:
			printf("@magicvar %d\n", in.magicvarval);
			break;
		case op_e:
			printf("@op %d\n", in.opval->op);
			break;
		case attribute_e:
			printf("@attribute %d\n", in.attributeval);
			break;
		case attributestr_e:
			printf("@attributestr\n");
			break;
	}
	return;
}
static void printvar_object(ehclassmember_t **in) {
	int i;
	ehclassmember_t * curr;
	for(i = 0; i < VARTABLE_S; i++) {
		curr = in[i];
		while(curr != NULL) {
			printf("%s <", curr->name);
			switch(curr->attribute.visibility) {
				case public_e:
					printf("public,");
					break;
				case private_e:
					printf("private,");
					break;
			}
			switch(curr->attribute.isstatic) {
				case static_e:
					printf("static,");
					break;
				case nonstatic_e:
					printf("non-static,");
					break;
			}
			switch(curr->attribute.isconst) {
				case const_e:
					printf("constant");
					break;
				case nonconst_e:
					printf("non-constant");
					break;
			}
			printf(">: ");
			printvar_retval(curr->value);
			curr = curr->next;
		}
	}
}
static void printvar_array(ehvar_t **in) {
	int i;
	ehvar_t *curr;

	for(i = 0; i < VARTABLE_S; i++) {
		curr = in[i];
		while(curr != NULL) {
			switch(curr->indextype) {
				case int_e:
					printf("%d: ", curr->index);
					break;
				case string_e:
					printf("'%s': ", curr->name);
					break;
				default:
					eh_error_type("array index", curr->indextype, eerror_e);
					break;
			}
			printvar_retval(curr->value);
			curr = curr->next;
		}
	}
}

/*
 * Type checking functions
 */
#define TYPEFUNC(typev) EHLIBFUNC(is_ ## typev) { \
	retval->type = bool_e; \
	ehretval_t *args = (ehretval_t *) Malloc(1 * sizeof(ehretval_t)); \
	if(eh_getargs(paras, 1, args, context, __FUNCTION__)) \
		EHLF_RETFALSE; \
	ehretval_t value = args[0]; \
	retval->boolval = (value.type == typev ## _e); \
	return; \
}

TYPEFUNC(null)
TYPEFUNC(int)
TYPEFUNC(string)
TYPEFUNC(bool)
TYPEFUNC(array)
TYPEFUNC(object)
// check whether a variable is a member of a specified class
EHLIBFUNC(class_is) {
	ehretval_t *args = (ehretval_t *) Malloc(2 * sizeof(ehretval_t));
	if(eh_getargs(paras, 2, args, context, __FUNCTION__))
		EHLF_RETFALSE;
	if(args[0].type != string_e) {
		eh_error_type("argument 0 to class_is", args[0].type, enotice_e);
		EHLF_RETFALSE;
	}
	if(args[1].type != object_e) {
		eh_error_type("argument 1 to class_is", args[1].type, enotice_e);
		EHLF_RETFALSE;
	}
	if(!strcmp(args[0].stringval, args[1].objectval->classname))
		EHLF_RETTRUE;
	else
		EHLF_RETFALSE;
}

/*
 * Including files
 */
ehretval_t eh_include_file(FILE *file);
EHLIBFUNC(include) {
	ehretval_t *args;
	FILE *infile;
	EHParser *parser;
	
	args = (ehretval_t *) Malloc(1 * sizeof(ehretval_t));
	if(eh_getargs(paras, 1, args, context, __FUNCTION__))
		EHLF_RETFALSE;
	if(args[0].type != string_e) {
		eh_error_type("argument 0 to include", args[0].type, enotice_e);
		EHLF_RETFALSE;
	}
	// do the work
	infile = fopen(args[0].stringval, "r");
	if(!infile) {
		eh_error("Unable to open included file", enotice_e);
		EHLF_RETFALSE;
	}
	parser = new EHParser();
	*retval = parser->parse_file(infile);
	// we're no longer returning
	returning = false;
	delete parser;
	return;
}
