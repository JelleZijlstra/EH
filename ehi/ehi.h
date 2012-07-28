/*
 * ehi.h
 * Jelle Zijlstra, December 2011
 */
#include "eh.bison.hpp"
#include "eh_error.h"

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

// symbol table for variables and functions
#define VARTABLE_S 1024

class EHI {
private:
	class EHParser *eval_parser;
	void init_eval_parser();
	// number of loops we're currently in
	int inloop;
	int breaking;
	int continuing;
	std::map<const std::string, ehcmd_t> cmdtable;
	
	// buffer for interactive prompt
	char *buffer;
	
	// our GC
	garbage_collector<ehretval_t> gc;
	
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
	ehretval_p eh_op_dot(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_try(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_catch(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_finally(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_always_execute(ehretval_p code, ehcontext_t context);
	ehretval_p perform_op(const char *name, const char *user_name, int nargs, ehretval_p *paras, ehcontext_t context);
	ehcmd_t get_command(const char *name);
	void insert_command(const char *name, const ehcmd_t cmd);
	void redirect_command(const char *redirect, const char *target);
	ehretval_p call_function(ehretval_p function, ehretval_p args, ehcontext_t context);
	void array_insert(eharray_t *array, ehretval_p in, int place, ehcontext_t context);
	ehretval_p eh_rangetoarray(const ehrange_t *const range);
	ehretval_p eh_xtoarray(ehretval_p in);
	ehretval_p eh_cast(const type_enum type, ehretval_p in, ehcontext_t context);

	ehobj_t *get_primitive_class(type_enum in) {
		return this->repo.get_object(in)->get_objectval();
	}

	// disallowed operations
	EHI(const EHI&) : eval_parser(), inloop(), breaking(), continuing(), cmdtable(), buffer(), gc(), returning(), repo(), global_object() {
		throw "Not allowed";
	}
	EHI operator=(const EHI&) {
		throw "Not allowed";
	}
public:
	ehretval_p eh_execute(ehretval_p node, const ehcontext_t context);
	void eh_setarg(int argc, char **argv);
	bool returning;
	type_repository repo;
	ehretval_p global_object;

	int eh_interactive(interactivity_enum interactivity = cli_prompt_e);
	ehretval_p parse_string(const char *cmd);
	ehretval_p parse_file(const char *name);
	EHI();
	void eh_exit(void);

	virtual ehretval_p execute_cmd(const char *rawcmd, eharray_t *paras);
	virtual char *eh_getline(class EHParser *parser = NULL);
	virtual ~EHI();
	
	// stuff for GC'ed ehretval_ts
#define EHRV_MAKE(ehtype, vtype) ehretval_p make_ ## ehtype(vtype in) { \
	ehretval_p out; \
	in->ehi = this; \
	this->gc.allocate(out); \
	ehretval_t::fill_ ## ehtype(out, in); \
	return out; \
}
	EHRV_MAKE(object, ehobj_t *)
	EHRV_MAKE(weak_object, ehobj_t *)
#undef ERHV_MAKE
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
	ehretval_p promote(ehretval_p in, ehcontext_t context);
	ehretval_p object_instantiate(ehobj_t *obj);
	ehretval_p call_method(ehretval_p in, const char *name, ehretval_p args, ehcontext_t context);

	// conversion methods, guaranteed to return the type they're supposed to return
#define CASTER(method_name, ehtype, fallback_value) ehretval_p to_ ## ehtype(ehretval_p in, ehcontext_t context) { \
	static ehretval_p fallback = ehretval_t::make_ ## ehtype(fallback_value); \
	ehretval_p out = call_method(in, #method_name, NULL, context); \
	if(out->type() == ehtype ## _e) { \
		return out; \
	} else { \
		eh_error(#method_name " does not return a " #ehtype, enotice_e); \
		return fallback; \
	} \
}
	CASTER(toString, string, strdup(""))
	CASTER(toInt, int, 0)
	CASTER(toFloat, float, 0.0)
	CASTER(toBool, bool, false)
#undef CASTER
	ehretval_p to_array(ehretval_p in, ehcontext_t context) {
		ehretval_p out = call_method(in, "toArray", NULL, context);
		if(out->type() == array_e) {
			return out;
		} else {
			eh_error("toArray does not return an array", enotice_e);
			eharray_t *arr = new eharray_t;
			arr->int_indices[0] = in;
			return this->make_array(arr);
		}
	}
	ehretval_p to_range(ehretval_p in, ehcontext_t context) {
		ehretval_p out = call_method(in, "toRange", NULL, context);
		if(out->type() == range_e) {
			return out;
		} else {
			eh_error("toRange does not return a range", enotice_e);
			ehrange_t *range = new ehrange_t(in, in);
			return this->make_range(range);
		}
	}
	ehretval_p to_tuple(ehretval_p in, ehcontext_t context) {
		ehretval_p out = call_method(in, "toTuple", NULL, context);
		if(out->type() == tuple_e) {
			return out;
		} else {
			eh_error("toTuple does not return a tuple", enotice_e);
			return this->make_tuple(new ehtuple_t(0, NULL));
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
		return _parent->eh_getline(this);
	}

	ehretval_p parse_file(FILE *infile);
	ehretval_p parse_string(const char *cmd);

	EHParser(interactivity_enum inter, EHI *parent) : _parent(parent), _scanner(), _interactivity(inter), buffer() {
		yylex_init_extra(this, &_scanner);
	}
	~EHParser(void) {
		yylex_destroy(_scanner);
	}
	EHI *_parent;
private:
	EHParser(const EHParser&) : _parent(), _scanner(), _interactivity(), buffer() {
		throw "Not allowed";
	}
	EHParser operator=(const EHParser&) {
		throw "Not allowed";
	}

	void *_scanner;
	interactivity_enum _interactivity;
	struct yy_buffer_state *buffer;

};
