#include "eh.h"

// Thread for the garbage collector. Before I actually start running this, I'll have to make sure the GC is thread-safe.
void *gc_thread(void *arg) {
	EHI *ehi = (EHI *)arg;
	while(1) {
		bool do_stop = ehi->gc.do_stop.get();
		if(do_stop) {
			pthread_exit(0);
		}
		ehi->gc.do_collect(ehi->global_object);
	}
}

int EHI::eh_interactive(interactivity_enum interactivity) {
	ehretval_p ret;

	EHParser parser(interactivity, this, ehcontext_t(this->global_object, this->global_object));
	while(1) {
		char *cmd = eh_getline(&parser);
		if(!cmd) {
			return eh_outer_exit(0);
		}
		// if a syntax error occurs, stop parsing and return -1
		try {
			ret = parser.parse_string(cmd);
		} catch(eh_exception &e) {
			handle_uncaught(e);
		}
	}
	// TODO: provide useful return type
	return 0;
}
ehretval_p EHI::parse_file(const char *name, ehcontext_t context) {
	FILE *infile = fopen(name, "r");
	if(!infile) {
		fprintf(stderr, "Could not open input file\n");
		return NULL;
	}
	EHParser parser(end_is_end_e, this, context);
	// if a syntax error occurs, stop parsing and return -1
	try {
		return parser.parse_file(infile);
	} catch(eh_exception &e) {
		handle_uncaught(e);
		return NULL;
	}
}
void EHI::init_eval_parser(ehcontext_t context) {
	if(eval_parser == NULL) {
		eval_parser = new EHParser(end_is_end_e, this, context);
	}
}
ehretval_p EHI::parse_string(const char *cmd, ehcontext_t context) {
	init_eval_parser(context);
	return eval_parser->parse_string(cmd);
}
