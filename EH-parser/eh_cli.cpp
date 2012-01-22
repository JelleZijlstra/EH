/*
 * eh_cli.c
 * Jelle Zijlstra, December 2011
 *
 * Default implementation of EH, used in the standalone interpreter.
 */
#include "eh.h"
#include "eh.bison.hpp"

extern FILE *yyin;
int yyparse(void);
struct yy_buffer_state *yy_scan_string ( const char *str );

EHI *interpreter;

void eh_usage(char *name) {
	fprintf(stderr, "Usage: %s\n\t%s file [arguments]\n\t%s -i\n\t%s -r code\n", name, name, name, name);
	exit(-1);
}

int main(int argc, char **argv) {
	ehretval_t ret;
	EHParser *parser;

	interpreter = new EHI;
	parser = new EHParser;

	try {
		if(argc == 1) {
			is_interactive = 1;
			ret.intval = interpreter->eh_interactive();
		}
		else if(!strcmp(argv[1], "-i")) {
			if(argc != 2)
				eh_usage(argv[0]);
			is_interactive = 2;
			ret.intval = interpreter->eh_interactive();
		}
		else if(!strcmp(argv[1], "-r")) {
			if(argc != 3)
				eh_usage(argv[0]);
			eh_init();
			ret = parser->parse_string(argv[2]);
		}
		else {
			FILE *infile = fopen(argv[1], "r");
			if(!infile)
				eh_error("Unable to open input file", efatal_e);
			eh_setarg(argc, argv);
			// set input
			eh_init();
			ret = parser->parse_file(infile);
		}
		delete interpreter;
		delete parser;
		exit(ret.intval);
	}
	catch(...) {
		exit(-1);
	}
}

ehretval_t EHI::execute_cmd(const char *name, ehvar_t **array) {
	eh_error("Use of commands outside of EH-PHP context", eerror_e);
	ehretval_t ret;
	ret.type = null_e;
	return ret;
}
EHI::~EHI(void) {
	return;
}
char *EHI::eh_getline(void) {
	return eh_getinput();
}
