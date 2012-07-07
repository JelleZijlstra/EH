/*
 * eh_cli.c
 * Jelle Zijlstra, December 2011
 *
 * Default implementation of EH, used in the standalone interpreter.
 */
#include "eh.h"
#include "eh_error.h"
#include "eh.bison.hpp"

void eh_usage(char *name) {
	fprintf(stderr, "Usage: %s\n\t%s file [arguments]\n\t%s -i\n\t%s -r code\n", name, name, name, name);
	exit(-1);
}

int main(int argc, char **argv) {
	ehretval_p ret;
	ret->type(int_e);

	EHI interpreter;

	try {
		if(argc == 1) {
			ret->intval = interpreter.eh_interactive(cli_no_prompt_e);
		} else if(!strcmp(argv[1], "-i")) {
			if(argc != 2) {
				eh_usage(argv[0]);
			}
			ret->intval = interpreter.eh_interactive();
		} else if(!strcmp(argv[1], "-r")) {
			if(argc != 3)
				eh_usage(argv[0]);
			ret = interpreter.parse_string(argv[2]);
		} else {
			interpreter.eh_setarg(argc, argv);
			ret = interpreter.parse_file(argv[1]);
		}
		return ret->intval;
	}
	catch(...) {
		return -1;
	}
}

ehretval_p EHI::execute_cmd(const char *name, eharray_t *paras) {
	eh_error("Use of commands outside of EH-PHP context", eerror_e);
	return NULL;
}
char *EHI::eh_getline(EHParser *parser) {
	if(parser->interactivity() == cli_prompt_e) {
		printf("> ");
	}
	char *buf = new char[512];
	return fgets(buf, 511, stdin);
}
