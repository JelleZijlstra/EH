#include "eh.hpp"
#include "std_lib/Attribute.hpp"
#include "std_lib/Node.hpp"
#include "eh.bison.hpp"
#include "eh.flex.hpp"

#include <stdio.h>

int yyparse(void *);

// Thread for the garbage collector. Before I actually start running this, I'll have to make sure the GC is thread-safe.
void *gc_thread(void *arg) {
	EHI *ehi = (EHI *)arg;
	EHInterpreter *parent = ehi->get_parent();
	while(1) {
		bool do_stop = parent->gc.do_stop.get();
		if(do_stop) {
			pthread_exit(0);
		}
		parent->gc.do_collect({parent->global_object});
	}
}

EHI::EHI(interactivity_enum _inter, EHInterpreter *_parent, ehcontext_t _context, const std::string &dir, const std::string &name) : scanner(), interactivity(_inter), yy_buffer(), buffer(), parent(_parent), interpreter_context(_context), code(nullptr), inloop(0), breaking(0), continuing(0), returning(false), working_dir(dir), context_name(name) {
	yylex_init_extra(this, &scanner);
}
EHI::EHI() : scanner(), interactivity(cli_prompt_e), yy_buffer(), buffer(), parent(nullptr), code(nullptr), inloop(0), breaking(0), continuing(0), returning(false), working_dir(eh_getcwd()), context_name("(none)") {
	yylex_init_extra(this, &scanner);
	parent = new EHInterpreter();
	interpreter_context = parent->global_object;
}
EHI::~EHI() {
	yylex_destroy(scanner);
	delete[] this->buffer;
}

int EHI::eh_interactive(interactivity_enum new_interactivity) {
	this->interactivity = new_interactivity;
	ehval_p ret = parse_interactive();
	return ret->is_a<Integer>() ? ret->get<Integer>() : 0;
}
ehval_p EHI::parse_interactive() {
	while(1) {
		char *cmd = eh_getline();
		if(cmd == nullptr) {
			return Integer::make(0);
		}
		try {
			parse_string(cmd);
		} catch(eh_exception &e) {
			handle_uncaught(e);
		} catch(quit_exception &) {
			return Integer::make(0);
		}
	}
}

ehval_p EHI::parse_file(const char *name, const ehcontext_t &context) {
	FILE *infile = fopen(name, "r");
	if(infile == nullptr) {
		fprintf(stderr, "Could not open input file\n");
		return nullptr;
	}
	EHI parser(end_is_end_e, this->get_parent(), context, eh_full_path(name), name);
	// if a syntax error occurs, stop parsing and return -1
	try {
		return parser.parse_file(infile);
	} catch(eh_exception &e) {
		handle_uncaught(e);
		return nullptr;
	}
}
ehval_p EHI::parse_string(const char *cmd, const ehcontext_t &context) {
	EHI parser(end_is_end_e, this->get_parent(), context, working_dir, "(eval'd code)");
	return parser.parse_string(cmd);
}

ehval_p EHI::parse_file(FILE *infile) {
	yy_buffer = yy_create_buffer(infile, YY_BUF_SIZE, scanner);
	yy_switch_to_buffer(yy_buffer, scanner);
	yyparse(scanner);

	// now execute the code
	const ehcontext_t context = get_context();
	if(parent->optimize) {
		code = optimize(code, context);
	}
	return eh_execute(code, context);
}

ehval_p EHI::parse_string(const char *cmd) {
	yy_switch_to_buffer(yy_scan_string(cmd, scanner), scanner);
	yyset_lineno(1, scanner);
	yyparse(scanner);
	return eh_execute(code, get_context());
}
