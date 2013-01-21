/*
 * eh_cli.c
 * Jelle Zijlstra, December 2011
 *
 * Default implementation of EH, used in the standalone interpreter.
 */
#include "eh.hpp"
#include "eh_files.hpp"

static void eh_usage(char *name) {
	fprintf(stderr, "Usage: %s\n\t%s file [arguments]\n\t%s -i\n\t%s -r code\n", name, name, name, name);
	exit(1);
}

int main(int argc, char **argv) {
	ehval_p ret;

	EHInterpreter interpreter;

	try {
		if(argc == 1) {
			EHI ehi(cli_no_prompt_e, &interpreter, interpreter.global_object, eh_getcwd(), "(none)");
			ret = ehi.parse_interactive();
		} else if(!strcmp(argv[1], "-i")) {
			if(argc != 2) {
				eh_usage(argv[0]);
			}
			EHI ehi(cli_prompt_e, &interpreter, interpreter.global_object, eh_getcwd(), "ehi -i");
			ret = ehi.parse_interactive();
		} else if(!strcmp(argv[1], "-r")) {
			if(argc != 3) {
				eh_usage(argv[0]);
			}
			EHI ehi(end_is_end_e, &interpreter, interpreter.global_object, eh_getcwd(), "ehi -r");
			try {
				ret = ehi.execute_string(argv[2]);
			} catch(eh_exception &e) {
				ehi.handle_uncaught(e);
				return 1;
			}
		} else {
			if(!strcmp(argv[1], "-O")) {
				interpreter.optimize = true;
				argc--;
				argv++;
			}
			interpreter.eh_setarg(argc, argv);
			EHI ehi(end_is_end_e, &interpreter, interpreter.global_object, eh_full_path(argv[1]), argv[1]);
			try {
				ret = ehi.execute_named_file(argv[1]);
			} catch(eh_exception &e) {
				ehi.handle_uncaught(e);
				return 1;
			}
		}
		//TODO: let scripts determine exit status
		return 0;
	} catch(...) {
		return 1;
	}
}
