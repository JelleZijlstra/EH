/*
 * eh_cli.c
 * Jelle Zijlstra, December 2011
 *
 * Default implementation of EH, used in the standalone interpreter.
 */
#include "eh.hpp"
#include "std_lib/UnknownCommandError.hpp"

static void eh_usage(char *name) {
	fprintf(stderr, "Usage: %s\n\t%s file [arguments]\n\t%s -i\n\t%s -r code\n", name, name, name, name);
	exit(-1);
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
				ret = ehi.parse_string(argv[2], interpreter.global_object);
			} catch(eh_exception &e) {
				ehi.handle_uncaught(e);
				return -1;
			}
		} else if(!strcmp(argv[1], "-O")) {
			interpreter.optimize = true;
			interpreter.eh_setarg(argc - 1, argv + 1);
			EHI ehi(end_is_end_e, &interpreter, interpreter.global_object, eh_full_path(argv[1]), argv[1]);
			ret = ehi.parse_file(argv[2], interpreter.global_object);
		} else {
			interpreter.eh_setarg(argc, argv);
			EHI ehi(end_is_end_e, &interpreter, interpreter.global_object, eh_full_path(argv[1]), argv[1]);
			ret = ehi.parse_file(argv[1], interpreter.global_object);
		}
		//TODO: let scripts determine exit status
		return 0;
	} catch(...) {
		return -1;
	}
}

ehval_p EHI::execute_cmd(const char *name, Array::t *paras) {
	throw_UnknownCommandError(name, this);
	return nullptr;
}
char *EHI::eh_getline() {
	if(this->buffer == nullptr) {
		this->buffer = new char[512];
	}
	if(get_interactivity() == cli_prompt_e) {
		printf("> ");
	}
	return fgets(buffer, 511, stdin);
}
