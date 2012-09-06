/*
 * ehi.h
 * Jelle Zijlstra, December 2011
 */
#include "eh.bison.hpp"
#include "eh_error.h"
#include "concurrency.h"

/*
 * Flex and Bison
 */
int yylex(YYSTYPE *, void *);
int yylex_init_extra(class EHParser *, void **);
int yylex_destroy(void *);
struct yy_buffer_state *yy_scan_string ( const char *str );

typedef enum {
	cli_prompt_e,
	cli_no_prompt_e,
	end_is_end_e
} interactivity_enum;

void *gc_thread(void *arg);

class EHI {
private:
	class EHParser *eval_parser;
	void init_eval_parser(ehcontext_t context);
	// number of loops we're currently in
	int inloop;
	int breaking;
	int continuing;
	std::map<const std::string, ehcmd_t> cmdtable;
	
	// buffer for interactive prompt
	char *buffer;
	
	void eh_init(void);
	// helper functions
	ehretval_p eh_op_command(const char *name, ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_for(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_while(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_as(opnode_t *op, ehcontext_t context);
	void eh_op_inherit(ehretval_p *paras, ehcontext_t context);
	void eh_op_continue(opnode_t *op, ehcontext_t context);
	void eh_op_break(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_array(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_anonclass(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_declareclosure(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_declareclass(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_tuple(ehretval_p node, ehcontext_t context);
	void eh_op_classmember(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_switch(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_given(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_colon(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_dollar(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_set(ehretval_p *paras, ehcontext_t context);
	ehretval_p set(ehretval_p lvalue, ehretval_p rvalue, ehcontext_t context);
	ehretval_p eh_op_dot(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_try(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_catch(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_finally(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_always_execute(ehretval_p code, ehcontext_t context);
	ehretval_p perform_op(const char *name, int nargs, ehretval_p *paras, ehcontext_t context);
	ehcmd_t get_command(const char *name);
	void insert_command(const char *name, const ehcmd_t cmd);
	void redirect_command(const char *redirect, const char *target);
	ehretval_p call_function(ehretval_p function, ehretval_p args, ehcontext_t context);
	void array_insert(eharray_t *array, ehretval_p in, int place, ehcontext_t context);
	ehretval_p eh_rangetoarray(const ehrange_t *const range);
	ehretval_p eh_xtoarray(ehretval_p in);
	ehretval_p eh_cast(const type_enum type, ehretval_p in, ehcontext_t context);

	// disallowed operations
	EHI(const EHI&);
	EHI operator=(const EHI&);
public:
	// our GC
	garbage_collector<ehretval_t> gc;
	bool returning;
	type_repository repo;
	ehretval_p global_object;

	ehretval_p get_primitive_class(type_enum in) {
		return this->repo.get_object(in);
	}

	ehretval_p eh_execute(ehretval_p node, const ehcontext_t context);
	void eh_setarg(int argc, char **argv);
	int eh_interactive(interactivity_enum interactivity = cli_prompt_e);
	ehretval_p parse_string(const char *cmd, ehcontext_t context);
	ehretval_p parse_file(const char *name, ehcontext_t context);
	EHI();
	void eh_exit(void);
	void handle_uncaught(eh_exception &e);
	ehmember_p set_property(ehretval_p object, const char *name, ehretval_p value, ehcontext_t context);
	ehmember_p set_member(ehretval_p object, const char *name, ehmember_p value, ehcontext_t context);
	ehretval_p get_property(ehretval_p object, const char *name, ehcontext_t context);

	virtual ehretval_p execute_cmd(const char *rawcmd, eharray_t *paras);
	virtual char *eh_getline(class EHParser *parser = NULL);
	virtual ~EHI();
	
	// stuff for GC'ed ehretval_ts
	ehretval_p make_object(ehobj_t *in) {
		ehretval_p out;
		in->ehi = this;
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
	ehretval_p promote(ehretval_p in, ehcontext_t context);
	ehretval_p call_method(ehretval_p in, const char *name, ehretval_p args, ehcontext_t context);

	// conversion methods, guaranteed to return the type they're supposed to return
#define CASTER(method_name, ehtype) ehretval_p to_ ## ehtype(ehretval_p in, ehcontext_t context) { \
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

	bool eh_floatequals(float infloat, ehretval_p operand2, ehcontext_t context) {
		ehretval_p operand = this->to_int(operand2, context);
		// checks whether a float equals an int. C handles this correctly.
		if(operand->type() != int_e) {
			return false;
		}
		return (infloat == operand->get_intval());
	}
};

/*
 * The EH parser
 */
class EHParser {
public:
	interactivity_enum interactivity() {
		return _interactivity;
	}
	char *getline() {
		return parent->eh_getline(this);
	}

	ehretval_p parse_file(FILE *infile);
	ehretval_p parse_string(const char *cmd);

	EHParser(interactivity_enum _inter, EHI *_parent, ehcontext_t _context) : parent(_parent), context(_context), scanner(), _interactivity(_inter), buffer() {
		yylex_init_extra(this, &scanner);
	}
	~EHParser(void) {
		yylex_destroy(scanner);
	}
	EHI *parent;
	ehcontext_t context;
private:
	EHParser(const EHParser&);
	EHParser operator=(const EHParser&);

	void *scanner;
	interactivity_enum _interactivity;
	struct yy_buffer_state *buffer;

};
