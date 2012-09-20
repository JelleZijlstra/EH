#include "eh.h"

// Thread for the garbage collector. Before I actually start running this, I'll have to make sure the GC is thread-safe.
void *gc_thread(void *arg) {
	EHI *ehi = (EHI *)arg;
	EHInterpreter *parent = ehi->get_parent();
	while(1) {
		bool do_stop = parent->gc.do_stop.get();
		if(do_stop) {
			pthread_exit(0);
		}
		parent->gc.do_collect(parent->global_object);
	}
}

int EHI::eh_interactive(interactivity_enum interactivity) {
	EHInterpreter *parent = this->get_parent();

	this->interactivity = interactivity;
	ehretval_p ret = parse_interactive();
	return (ret->type() == int_e) ? ret->get_intval() : 0;
}
ehretval_p EHI::parse_interactive() {
	while(1) {
		char *cmd = eh_getline();
		if(!cmd) {
			return eh_outer_exit(0);
		}
		try {
			parse_string(cmd);
		} catch(eh_exception &e) {
			handle_uncaught(e);
		} catch(quit_exception &e) {
			return ehretval_t::make_int(0);
		}
	}
	return ehretval_t::make_int(0);
}

ehretval_p EHI::parse_file(const char *name, ehcontext_t context) {
	FILE *infile = fopen(name, "r");
	if(!infile) {
		fprintf(stderr, "Could not open input file\n");
		return NULL;
	}
	EHI parser(end_is_end_e, this->get_parent(), context);
	// if a syntax error occurs, stop parsing and return -1
	try {
		return parser.parse_file(infile);
	} catch(eh_exception &e) {
		handle_uncaught(e);
		return NULL;
	}
}
ehretval_p EHI::parse_string(const char *cmd, ehcontext_t context) {
	EHI parser(end_is_end_e, this->get_parent(), context);
	return parser.parse_string(cmd);
}
