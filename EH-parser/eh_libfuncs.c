/*
 * eh_libfuncs.c
 * Jelle Zijlstra, December 2011
 *
 * Contains definitions of EH library functions
 */
#include "eh_interpreter.h"

static void printvar_retval(ehretval_t in);
static void printvar_var(ehvar_t *in);
static void printvar_array(ehvar_t **in);

EHLIBFUNC(getinput) {
	retval->type = int_e;
	fscanf(stdin, "%d", &retval->intval);
	return;
}

EHLIBFUNC(printvar) {
	// this function always returns NULL
	retval->type = null_e;

	if(paras[0].type == opnode_e && paras[0].op.op == ',') {
		fprintf(stderr, "Incorrect argument count for function printvar\n");
		return;
	}
	printvar_retval(execute(&paras[0]));
	return;
}

static void printvar_retval(ehretval_t in) {
	switch(in.type) {
		case null_e:
			printf("null\n");
			break;
		case int_e:
			printf("@int %d\n", in.intval);
			break;
		case string_e:
			printf("@string '%s'\n", in.strval);
			break;
		case array_e:
			printf("@array [\n");
			printvar_array(in.arrval);
			printf("]\n");
			break;
		case bool_e:
			if(in.boolval)
				printf("@bool true\n");
			else
				printf("@bool false\n");
			break;
		default:
			fprintf(stderr, "Unsupported data type\n");
			break;
	}
	return;
}
static void printvar_var(ehvar_t *in) {
	switch(in->type) {
		case null_e:
			printf("null\n");
			break;
		case int_e:
			printf("@int %d\n", in->intval);
			break;
		case string_e:
			printf("@string '%s'\n", in->strval);
			break;
		case bool_e:
			if(in->boolval)
				printf("@bool true\n");
			else
				printf("@bool false\n");
			break;
		case array_e:
			printf("@array [\n");
			printvar_array(in->arrval);
			printf("]\n");
			break;
		default:
			fprintf(stderr, "Unsupported data type\n");
			break;
	}
	return;
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
			}
			printvar_var(curr);
			curr = curr->next;
		}
	}
}