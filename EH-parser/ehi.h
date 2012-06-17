/*
 * ehi.h
 * Jelle Zijlstra, December 2011
 */
#include "eh.bison.hpp"

/*
 * Flex and Bison
 */
int yylex(YYSTYPE *, void *);
int yylex_init_extra(struct EHParser *, void **);
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
	ehvar_t *vartable[VARTABLE_S];
	ehclass_t *classtable[VARTABLE_S];
	ehcmd_bucket_t *cmdtable[VARTABLE_S];
	
	// current object, gets passed around
	ehcontext_t newcontext;
	ehscope_t global_scope;
	ehscope_t *curr_scope;
	
	void eh_init(void);
	// helper functions
	ehretval_t *eh_op_command(const char *name, ehretval_t *node, ehcontext_t context);
	ehretval_t *eh_op_for(opnode_t *op, ehcontext_t context);
	ehretval_t *eh_op_while(ehretval_t **paras, ehcontext_t context);
	ehretval_t *eh_op_as(opnode_t *op, ehcontext_t context);
	ehretval_t *eh_op_new(const char *name);
	void eh_op_continue(opnode_t *op, ehcontext_t context);
	void eh_op_break(opnode_t *op, ehcontext_t context);
	ehretval_t *eh_op_array(ehretval_t *node, ehcontext_t context);
	ehretval_t *eh_op_anonclass(ehretval_t *node, ehcontext_t context);
	void eh_op_declarefunc(ehretval_t **paras);
	ehretval_t *eh_op_declareclosure(ehretval_t **paras);
	void eh_op_declareclass(ehretval_t **paras, ehcontext_t context);
	ehretval_t *eh_op_switch(ehretval_t **paras, ehcontext_t context);
	ehretval_t *eh_op_given(ehretval_t **paras, ehcontext_t context);
	ehretval_t *eh_op_colon(ehretval_t **paras, ehcontext_t context);
	ehretval_t *eh_op_reference(opnode_t *op, ehcontext_t context);
	ehretval_t **eh_op_lvalue(opnode_t *op, ehcontext_t context);
	ehretval_t *eh_op_dollar(ehretval_t *node, ehcontext_t context);
	void eh_op_set(ehretval_t **paras, ehcontext_t context);
	ehretval_t *eh_op_accessor(ehretval_t **paras, ehcontext_t context);
	ehcmd_t *get_command(const char *name);
	void insert_command(const ehcmd_t cmd);
	void redirect_command(const char *redirect, const char *target);

	// prototypes
	bool insert_variable(ehvar_t *var);
	ehvar_t *get_variable(const char *name, ehscope_t *scope, ehcontext_t context, int token);
	void remove_variable(const char *name, ehscope_t *scope);
	void list_variables(void);
	ehretval_t *call_function(ehfm_t *f, ehretval_t *args, ehcontext_t context, ehcontext_t newcontext);
	ehretval_t *call_function_args(ehfm_t *f, const ehcontext_t context, const ehcontext_t newcontext, const int nargs, ehretval_t *args);
	void remove_scope(ehscope_t *scope);
	void array_insert(ehvar_t **array, ehretval_t *in, int place, ehcontext_t context);
	void insert_class(ehclass_t *classobj);
	ehclass_t *get_class(const char *name);
	void class_insert(ehvar_t **classarr, const ehretval_t *in, ehcontext_t context);
	ehretval_t **object_access(ehretval_t *name, ehretval_t *index, ehcontext_t context, int token);
	ehretval_t **colon_access(ehretval_t *operand1, ehretval_t *index, ehcontext_t context, int token);
public:
	ehretval_t *eh_execute(ehretval_t *node, const ehcontext_t context);
	void eh_setarg(int argc, char **argv);
	bool returning;

	int eh_interactive(interactivity_enum interactivity = cli_prompt_e);
	ehretval_t parse_string(const char *cmd);
	ehretval_t parse_file(const char *name);
	EHI();
	void eh_exit(void);

	virtual ehretval_t *execute_cmd(const char *rawcmd, ehvar_t **paras);
	virtual char *eh_getline(class EHParser *parser = NULL);
	virtual ~EHI();
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

	ehretval_t parse_file(FILE *infile);
	ehretval_t parse_string(const char *cmd);

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
