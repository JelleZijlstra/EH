#include "eh.h"

int EHI::eh_interactive(interactivity_enum interactivity) {
	ehretval_p ret;

	EHParser parser(interactivity, this);
	char *cmd = eh_getline(&parser);
	if(!cmd) {
		return eh_outer_exit(0);
	}
	// if a syntax error occurs, stop parsing and return -1
	try {
		ret = parser.parse_string(cmd);
	} catch(...) {
		// do nothing
	}
	return ret->intval;
}
ehretval_p EHI::parse_file(const char *name) {
	FILE *infile = fopen(name, "r");
	if(!infile) {
		fprintf(stderr, "Could not open input file\n");
		return NULL;
	}
	EHParser parser(end_is_end_e, this);
	// if a syntax error occurs, stop parsing and return -1
	try {
		return parser.parse_file(infile);
	} catch(...) {
		// TODO: actually do something useful with exceptions
		return NULL;
	}
}
void EHI::init_eval_parser() {
	if(eval_parser == NULL) {
		eval_parser = new EHParser(end_is_end_e, this);
	}
}
ehretval_p EHI::parse_string(const char *cmd) {
	init_eval_parser();
	return eval_parser->parse_string(cmd);
}
