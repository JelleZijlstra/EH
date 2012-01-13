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
int main(int argc, char **argv) {
	ehretval_t ret;
	EHParser *parser;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s file\n\t%s -i\n\t%s -r code", argv[0], argv[0], argv[0]);
		exit(-1);
	}
	interpreter = new EHI;
	parser = new EHParser;

	try {
		if(!strcmp(argv[1], "-i")) {
			ret.intval = interpreter->eh_interactive();
		}
		else if(!strcmp(argv[1], "-r")) {
			if(argc < 3) {
				fprintf(stderr, "Usage: %s file\n\t%s -i\n\t%s -r code", argv[0], argv[0], argv[0]);
				eh_error(NULL, efatal_e);
			}
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
	catch(std::exception &e) {
		exit(-1);
	}
	catch(std::exception *e) {
		exit(-1);
	}
}

int EHI::execute_cmd(char *name, ehvar_t **array) {
	eh_error("Use of commands outside of EH-PHP context", eerror_e);
	return 0;
}
EHI::~EHI(void) {
	return;
}
char *EHI::eh_getline(void) {
	return eh_getinput();
}
