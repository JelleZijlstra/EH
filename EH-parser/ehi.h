/*
 * ehi.h
 * Jelle Zijlstra, December 2011
 */
#include "eh.bison.hpp"

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
	
	// hack: used to implement several forms of inter-method communication
	bool is_strange_arrow;
	
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
	ehretval_p eh_op_new(ehretval_p *paras, ehcontext_t context);
	void eh_op_inherit(ehretval_p *paras, ehcontext_t context);
	void eh_op_continue(opnode_t *op, ehcontext_t context);
	void eh_op_break(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_array(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_anonclass(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_declareclosure(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_declareclass(opnode_t *op, ehcontext_t context);
	void eh_op_classmember(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_switch(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_given(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_colon(ehretval_p *paras, ehcontext_t context);
	ehretval_p &eh_op_lvalue(opnode_t *op, ehcontext_t context);
	ehretval_p eh_op_dollar(ehretval_p node, ehcontext_t context);
	void eh_op_set(ehretval_p *paras, ehcontext_t context);
	ehretval_p eh_op_accessor(ehretval_p *paras, ehcontext_t context);
	ehcmd_t get_command(const char *name);
	void insert_command(const char *name, const ehcmd_t cmd);
	void redirect_command(const char *redirect, const char *target);

	// prototypes
	ehretval_p call_function(ehobj_t *obj, ehretval_p args, ehcontext_t context);
	ehretval_p call_function_args(ehobj_t *obj, const int nargs, ehretval_p args[], ehcontext_t context);
	void array_insert(eharray_t *array, ehretval_p in, int place, ehcontext_t context);
	ehretval_p &object_access(ehretval_p name, ehretval_p index, ehcontext_t context, int token);
	ehretval_p &colon_access(ehretval_p operand1, ehretval_p index, ehcontext_t context, int token);
	ehretval_p object_instantiate(ehobj_t *obj);
	ehobj_t *get_class(ehretval_p code, ehcontext_t context);
  ehretval_p eh_rangetoarray(const ehrange_t *const range);
  ehretval_p eh_xtoarray(ehretval_p in);
  ehretval_p eh_cast(const type_enum type, ehretval_p in);

public:
	ehretval_p eh_execute(ehretval_p node, const ehcontext_t context);
	void eh_setarg(int argc, char **argv);
	bool returning;
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
  this->gc.allocate(out); \
  ehretval_t::fill_ ## ehtype(out, in); \
  return out; \
}
  EHRV_MAKE(object, ehobj_t *)
  EHRV_MAKE(weak_object, ehobj_t *)
  EHRV_MAKE(func, ehobj_t *)
  EHRV_MAKE(array, eharray_t *)
#undef ERHV_MAKE
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

	EHParser(interactivity_enum inter, EHI *parent) :  _parent(parent), _interactivity(inter) {
		yylex_init_extra(this, &_scanner);
	}
	~EHParser(void) {
		yylex_destroy(_scanner);
	}
	EHI *_parent;
private:
	void *_scanner;
	interactivity_enum _interactivity;
	struct yy_buffer_state *buffer;

};
