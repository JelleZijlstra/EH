/*
 * eh_cli.c
 * Jelle Zijlstra, December 2011
 *
 * Default implementation of EH, used in the standalone interpreter.
 */
#include "eh.h"

extern FILE *yyin;
void yyparse(void);

EHI *interpreter;
int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s file\n\t%s -i\n", argv[0], argv[0]);
		eh_error(NULL, efatal_e);
	}
	interpreter = new EHI;
	if(strcmp(argv[1], "-i")) {
		FILE *infile = fopen(argv[1], "r");
		if(!infile)
			eh_error("Unable to open input file", efatal_e);
		eh_setarg(argc, argv);
		// set input
		yyin = infile;
		eh_init();
		yyparse();
	}
	else {
		interpreter->eh_interactive();
	}
	delete interpreter;
	return 0;
}

int EHI::execute_cmd(char *name, ehvar_t **array) {
	eh_error("Use of commands outside of EH-PHP context", eerror_e);
	return 0;
}
EHI::~EHI(void) {
	return;
}