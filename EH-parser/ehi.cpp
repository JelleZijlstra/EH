#include "eh.h"

int EHI::eh_interactive(interactivity_enum interactivity) {
	ehretval_t ret;

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
	return ret.intval;
}
ehretval_t EHI::parse_file(const char *name) {
	ehretval_t ret(NULL);
	FILE *infile = fopen(name, "r");
	if(!infile) {
		fprintf(stderr, "Could not open input file\n");
		return ret;
	}
	EHParser parser(end_is_end_e, this);
	// if a syntax error occurs, stop parsing and return -1
	try {
		return parser.parse_file(infile);
	} catch(...) {
		// TODO: actually do something useful with exceptions
		return ret;
	}
}
void EHI::init_eval_parser() {
	if(eval_parser == NULL) {
		eval_parser = new EHParser(end_is_end_e, new EHI);
	}
}
ehretval_t EHI::parse_string(const char *cmd) {
	init_eval_parser();
	return eval_parser->parse_string(cmd);
}

