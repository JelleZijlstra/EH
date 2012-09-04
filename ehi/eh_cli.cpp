/*
 * eh_cli.c
 * Jelle Zijlstra, December 2011
 *
 * Default implementation of EH, used in the standalone interpreter.
 */
#include "eh.h"
#include "eh.bison.hpp"
#include "std_lib/UnknownCommandError.h"

void eh_usage(char *name) {
	fprintf(stderr, "Usage: %s\n\t%s file [arguments]\n\t%s -i\n\t%s -r code\n", name, name, name, name);
	exit(-1);
}

int main(int argc, char **argv) {
	ehretval_p ret;

	EHI interpreter;

	try {
		if(argc == 1) {
			ret = ehretval_t::make_int(interpreter.eh_interactive(cli_no_prompt_e));
		} else if(!strcmp(argv[1], "-i")) {
			if(argc != 2) {
				eh_usage(argv[0]);
			}
			ret = ehretval_t::make_int(interpreter.eh_interactive());
		} else if(!strcmp(argv[1], "-r")) {
			if(argc != 3) {
				eh_usage(argv[0]);
			}
			try {
				ret = interpreter.parse_string(argv[2], interpreter.global_object);
			} catch(eh_exception &e) {
				interpreter.handle_uncaught(e);
				return -1;
			}
		} else {
			interpreter.eh_setarg(argc, argv);
			ret = interpreter.parse_file(argv[1], interpreter.global_object);
		}
		//TODO: let scripts determine exit status
		return 0;
	}
	catch(...) {
		return -1;
	}
}

ehretval_p EHI::execute_cmd(const char *name, eharray_t *paras) {
	throw_UnknownCommandError(name, this);
	return NULL;
}
char *EHI::eh_getline(EHParser *parser) {
	if(this->buffer == NULL) {
		this->buffer = new char[512];	
	}
	if(parser->interactivity() == cli_prompt_e) {
		printf("> ");
	}
	return fgets(buffer, 511, stdin);
}
