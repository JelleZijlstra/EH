/*
 * ehi.h
 * Jelle Zijlstra, December 2011
 */
#include "eh.bison.hpp"
#include "eh_error.h"
#include "concurrency.h"
#include "std_lib/Function.h"

/*
 * Flex and Bison
 */
int yylex(YYSTYPE *, void *);
int yylex_init_extra(class EHI *, void **);
int yylex_destroy(void *);
struct yy_buffer_state *yy_scan_string (const char *str);

typedef enum {
	cli_prompt_e,
	cli_no_prompt_e,
	end_is_end_e
} interactivity_enum;

void *gc_thread(void *arg);

class EHInterpreter {
public:
	ehhash_t *cmdtable;

private:	
	void eh_init(void);

	// disallowed operations
	EHInterpreter(const EHInterpreter&);
	EHInterpreter operator=(const EHInterpreter&);
public:
	// our GC
	garbage_collector<ehretval_t> gc;
	type_repository repo;
	ehretval_p global_object;
	ehretval_p function_object;
	ehretval_p base_object;

	ehretval_p get_primitive_class(type_enum in) {
		return this->repo.get_object(in);
	}

	void eh_setarg(int argc, char **argv);
	EHInterpreter();
	void eh_exit(void);

	virtual ~EHInterpreter();

	// helper functions
	ehretval_p get_command(const char *name);
	void insert_command(const char *name, ehretval_p cmd);
	void redirect_command(const char *redirect, const char *target);	
	ehretval_p make_method(ehlibmethod_t in, ehretval_p function_object);

	// stuff for GC'ed ehretval_ts
	ehretval_p make_object(ehobj_t *in) {
		ehretval_p out;
		this->gc.allocate(out);
		ehretval_t::fill_object(out, in);
		return out;
	}
	ehretval_p make_binding(ehbinding_t *in) {
		ehretval_p out;
		this->gc.allocate(out);
		ehretval_t::fill_binding(out, in);
		return out;
	}
	ehretval_p make_range(ehrange_t *in) {
		ehretval_p out;
		this->gc.allocate(out);
		ehretval_t::fill_range(out, in);
		return out;
	}
	ehretval_p make_array(eharray_t *in) {
		ehretval_p out;
		this->gc.allocate(out);
		ehretval_t::fill_array(out, in);
		return out;
	}
	ehretval_p make_hash(ehhash_t *in) {
		ehretval_p out;
		this->gc.allocate(out);
		ehretval_t::fill_hash(out, in);
		return out;
	}
	ehretval_p make_tuple(ehtuple_t *in) {
		ehretval_p out;
		this->gc.allocate(out);
		ehretval_t::fill_tuple(out, in);
		return out;
	}
	ehretval_p make_super_class(ehsuper_t *in) {
		ehretval_p out;
		this->gc.allocate(out);
		ehretval_t::fill_super_class(out, in);
		return out;
	}
};

/*
 * The EH parser
 */
class EHI {
public:

	ehretval_p eh_execute(ehretval_p node, const ehcontext_t context);

	virtual char *eh_getline();
	virtual ehretval_p execute_cmd(const char *rawcmd, eharray_t *paras);

	ehretval_p parse_file(FILE *infile);
	ehretval_p parse_string(const char *cmd);

	int eh_interactive(interactivity_enum interactivity = cli_prompt_e);
	ehretval_p global_parse_file(const char *name) {
		return parse_file(name, parent->global_object);
	}
	ehretval_p global_parse_string(const char *cmd) {
		return parse_string(cmd, parent->global_object);
	}
	ehretval_p parse_string(const char *cmd, ehcontext_t context);
	ehretval_p parse_file(const char *name, ehcontext_t context);
	ehretval_p parse_interactive();

	ehretval_p get_property(ehretval_p object, const char *name, ehcontext_t context);
	ehretval_p call_method(ehretval_p in, const char *name, ehretval_p args, ehcontext_t context);
	ehmember_p set_property(ehretval_p object, const char *name, ehretval_p value, ehcontext_t context);
	void handle_uncaught(eh_exception e);

	/*
	 * Accessors
	 */
	interactivity_enum get_interactivity() const {
		return interactivity;
	}
	bool get_returning() const {
		return returning;
	}
	void not_returning() {
		this->returning = false;
	}
	EHInterpreter *get_parent() const {
		return parent;
	}
	ehcontext_t get_context() const {
		return context;
	}
	ehretval_p global() const {
		return parent->global_object;
	}

	/*
	 * Constructors and destructors.
	 */
	EHI(interactivity_enum _inter, EHInterpreter *_parent, ehcontext_t _context) : scanner(), interactivity(_inter), yy_buffer(), buffer(), parent(_parent), context(_context), inloop(0), breaking(0), continuing(0), returning(false) {
		yylex_init_extra(this, &scanner);
	}
	EHI() : scanner(), interactivity(cli_prompt_e), yy_buffer(), buffer(), parent(NULL), inloop(0), breaking(0), continuing(0), returning(false) {
		yylex_init_extra(this, &scanner);
		parent = new EHInterpreter();
		context = parent->global_object;
	}
	~EHI(void) {
		yylex_destroy(scanner);
		delete[] this->buffer;
	}
private:

	/*
	 * Properties
	 */
	void *scanner;
	interactivity_enum interactivity;
	struct yy_buffer_state *yy_buffer;
	// buffer for interactive prompt
	char *buffer;
	EHInterpreter *parent;
	ehcontext_t context;

	// number of loops we're currently in
	int inloop;
	int breaking;
	int continuing;
	bool returning;
	
	/*
	 * Disallowed operations
	 */
	EHI(const EHI&);
	EHI operator=(const EHI&);

	/*
	 * Private methods
	 */
	ehmember_p set_member(ehretval_p object, const char *name, ehmember_p value, ehcontext_t context);
	ehretval_p call_function(ehretval_p function, ehretval_p args, ehcontext_t context);
	ehretval_p eh_always_execute(ehretval_p code, ehcontext_t context);
	ehretval_p eh_cast(const type_enum type, ehretval_p in, ehcontext_t context);
	ehretval_p eh_op_anonclass(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_array(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_colon(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_command(const char *name, ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_declareclass(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_declareclosure(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_dollar(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_dot(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_for(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_given(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_in(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_set(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_switch(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_try(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_tuple(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_while(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_rangetoarray(const ehrange_t *const range);
	ehretval_p eh_try_catch(ehretval_p try_block, ehretval_p catch_blocks, ehcontext_t context);
	ehretval_p eh_xtoarray(ehretval_p in);
	ehretval_p perform_op(const char *name, int nargs, ehretval_p *paras, ehcontext_t context);
	ehretval_p set(ehretval_p lvalue, ehretval_p rvalue, ehcontext_t context);
	void array_insert(eharray_t *array, ehretval_p in, int place, ehcontext_t context);
	void eh_op_break(opnode_t *op, ehcontext_t context);
	void eh_op_classmember(opnode_t *op, ehcontext_t context);
	void eh_op_continue(opnode_t *op, ehcontext_t context);

	ehretval_p promote(ehretval_p in, ehcontext_t context);
	bool eh_floatequals(float infloat, ehretval_p operand2, ehcontext_t context) {
		ehretval_p operand = this->to_int(operand2, context);
		// checks whether a float equals an int. C handles this correctly.
		if(operand->type() != int_e) {
			return false;
		}
		return (infloat == operand->get_intval());
	}

public:
	// conversion methods, guaranteed to return the type they're supposed to return
#define CASTER(method_name, ehtype) ehretval_p to_ ## ehtype(ehretval_p in, ehcontext_t context) { \
	if(in->type() == ehtype ## _e) { \
		return in; \
	} \
	ehretval_p out = call_method(in, #method_name, NULL, context); \
	if(out->type() == ehtype ## _e) { \
		return out; \
	} else { \
		throw_TypeError(#method_name " must return a " #ehtype, out->type(), this); \
		return NULL; \
	} \
}
	CASTER(toString, string)
	CASTER(toInt, int)
	CASTER(toFloat, float)
	CASTER(toBool, bool)
#undef CASTER
	ehretval_p to_array(ehretval_p in, ehcontext_t context) {
		ehretval_p out = call_method(in, "toArray", NULL, context);
		if(out->type() == array_e) {
			return out;
		} else {
			throw_TypeError("toArray must return an array", out->type(), this);
			return NULL;
		}
	}
	ehretval_p to_range(ehretval_p in, ehcontext_t context) {
		ehretval_p out = call_method(in, "toRange", NULL, context);
		if(out->type() == range_e) {
			return out;
		} else {
			throw_TypeError("toRange must return a range", out->type(), this);
			return NULL;
		}
	}
	ehretval_p to_tuple(ehretval_p in, ehcontext_t context) {
		ehretval_p out = call_method(in, "toTuple", NULL, context);
		if(out->type() == tuple_e) {
			return out;
		} else {
			throw_TypeError("toTuple must return a tuple", out->type(), this);
			return NULL;
		}
	}
};
