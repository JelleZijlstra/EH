#include "eh.hpp"
#include "std_lib/ArgumentError.hpp"
#include "std_lib/Attribute.hpp"
#include "std_lib/Node.hpp"
#include "std_lib/String.hpp"
#include "eh.bison.hpp"
#include "eh.flex.hpp"

#include <stdio.h>

int yyparse(void *);

EHI::EHI(interactivity_enum _inter, EHInterpreter *_parent, ehcontext_t _context, const std::string &dir, const std::string &name) : scanner(), interactivity(_inter), yy_buffer(), buffer(), parent(_parent), interpreter_context(_context), program_code(nullptr), inloop(0), breaking(0), continuing(0), returning(false), working_dir(dir), context_name(name) {
	yylex_init_extra(this, &scanner);
}
EHI::EHI() : scanner(), interactivity(cli_prompt_e), yy_buffer(), buffer(), parent(nullptr), program_code(nullptr), inloop(0), breaking(0), continuing(0), returning(false), working_dir(eh_getcwd()), context_name("(none)") {
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
			execute_string(cmd);
		} catch(eh_exception &e) {
			handle_uncaught(e);
		} catch(quit_exception &) {
			return Integer::make(0);
		}
	}
}

ehval_p EHI::spawning_parse_file(const char *name, const ehcontext_t &context) {
	EHI parser(end_is_end_e, this->get_parent(), context, eh_full_path(name), name);
	try {
		return parser.execute_named_file(name);
	} catch(eh_exception &e) {
		handle_uncaught(e);
		return nullptr;
	}
}
ehval_p EHI::spawning_parse_string(const char *cmd, const ehcontext_t &context) {
	EHI parser(end_is_end_e, this->get_parent(), context, working_dir, "(eval'd code)");
	return parser.execute_string(cmd);
}

void EHI::do_parse() {
	parent->gc.stop_collecting();
	// std::cout << "Turned off GC" << std::endl;
	yyparse(scanner);
	parent->gc.resume_collecting();
	// std::cout << "Turned on GC" << std::endl;
}

void EHI::parse_file(FILE *infile) {
	yy_buffer = yy_create_buffer(infile, YY_BUF_SIZE, scanner);
	yy_switch_to_buffer(yy_buffer, scanner);
	do_parse();
}

void EHI::parse_string(const char *cmd) {
	yy_switch_to_buffer(yy_scan_string(cmd, scanner), scanner);
	yyset_lineno(1, scanner);
	do_parse();
}

ehval_p EHI::execute_code() {
	const ehcontext_t context = get_context();
	if(parent->optimize) {
		program_code = optimize(program_code, context);
	}
	return eh_execute(program_code, context);
}

ehval_p EHI::execute_file(FILE *infile) {
	parse_file(infile);
	return execute_code();
}

ehval_p EHI::execute_named_file(const char *name) {
	FILE *infile = fopen(name, "r");
	if(infile == nullptr) {
		throw_ArgumentError("Could not open input file", "EH core", String::make(strdup(name)), this);
	}
	return execute_file(infile);
}

ehval_p EHI::execute_string(const char *cmd) {
	parse_string(cmd);
	return execute_code();
}

