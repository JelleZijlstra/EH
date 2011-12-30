/*
 * eh_libfuncs.c
 * Jelle Zijlstra, December 2011
 *
 * Contains definitions of EH library functions
 */
#include "eh_interpreter.h"

static void printvar_retval(ehretval_t in);
static void printvar_array(ehvar_t **in);
static void printvar_object(ehclassmember_t **in);

EHLIBFUNC(getinput) {
	retval->type = int_e;
	fscanf(stdin, "%d", &retval->intval);
	return;
}

EHLIBFUNC(printvar) {
	// this function always returns NULL
	retval->type = null_e;

	if(paras->opval->paras[0]->opval->nparas != 0) {
		eh_error_argcount_lib("printvar", 1, 2);
		return;
	}
	printvar_retval(execute(paras->opval->paras[1], context));
	return;
}

static void printvar_retval(ehretval_t in) {
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
			printf("@object <%s> [\n", in.objectval->class);
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