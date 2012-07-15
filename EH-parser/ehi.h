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
  // cache of commonly used classes
  struct {
    ehobj_t *Integer;
    ehobj_t *Float;
    ehobj_t *Bool;
    ehobj_t *Null;
    ehobj_t *String;
    ehobj_t *Array;
    ehobj_t *Range;
  } cache;

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
	ehretval_p eh_op_dollar(ehretval_p node, ehcontext_t context);
	ehretval_p eh_op_set(ehretval_p *paras, ehcontext_t context);
  ehretval_p eh_op_tilde(ehretval_p in, ehcontext_t context);
  ehretval_p eh_op_uminus(ehretval_p in, ehcontext_t context);
  ehretval_p eh_op_dot(ehretval_p *paras, ehcontext_t context);
  ehretval_p eh_looseequals(ehretval_p operand1, ehretval_p operand2, ehcontext_t context);
  ehretval_p perform_op(const char *name, const char *user_name, ehretval_p *paras, ehcontext_t context);
	ehcmd_t get_command(const char *name);
	void insert_command(const char *name, const ehcmd_t cmd);
	void redirect_command(const char *redirect, const char *target);
	ehretval_p call_function(ehobj_t *obj, ehretval_p object_data, ehretval_p args, ehcontext_t context);
	ehretval_p call_function_args(ehobj_t *obj, ehretval_p object_data, const int nargs, ehretval_p args[], ehcontext_t context);
	void array_insert(eharray_t *array, ehretval_p in, int place, ehcontext_t context);
	ehretval_p &object_access(ehretval_p name, ehretval_p index, ehcontext_t context, int token);
	ehretval_p &colon_access(ehretval_p operand1, ehretval_p index, ehcontext_t context, int token);
	ehretval_p object_instantiate(ehobj_t *obj, ehcontext_t context);
	ehobj_t *get_class(ehretval_p code, ehcontext_t context);
  ehretval_p eh_rangetoarray(const ehrange_t *const range);
  ehretval_p eh_xtoarray(ehretval_p in);
  ehretval_p eh_cast(const type_enum type, ehretval_p in, ehcontext_t context);

  ehobj_t *get_primitive_class(type_enum in) {
    switch(in) {
      case int_e: return this->cache.Integer;
      case float_e: return this->cache.Float;
      case bool_e: return this->cache.Bool;
      case null_e: return this->cache.Null;
      case string_e: return this->cache.String;
      case array_e: return this->cache.Array;
      default: return NULL;
    }
  }

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
  in->ehi = this; \
  this->gc.allocate(out); \
  ehretval_t::fill_ ## ehtype(out, in); \
  return out; \
}
  EHRV_MAKE(object, ehobj_t *)
  EHRV_MAKE(weak_object, ehobj_t *)
  EHRV_MAKE(func, ehobj_t *)
#undef ERHV_MAKE
  ehretval_p make_binding(ehbinding_t *in) {
    ehretval_p out;
    this->gc.allocate(out);
    ehretval_t::fill_binding(out, in);
    return out;
  }
  ehretval_p make_array(eharray_t *in) {
    ehretval_p out;
    this->gc.allocate(out);
    ehretval_t::fill_array(out, in);
    return out;
  }
  ehretval_p promote(ehretval_p in, ehcontext_t context);
  ehretval_p call_method(ehretval_p in, const char *name, int nargs, ehretval_p *args, ehcontext_t context);
  ehretval_p call_method_obj(ehobj_t *obj, const char *name, int nargs, ehretval_p *args, ehcontext_t context);

  // conversion methods, guaranteed to return the type they're supposed to return
#define CASTER(method_name, ehtype, fallback_value) ehretval_p to_ ## ehtype(ehretval_p in, ehcontext_t context) { \
  static ehretval_p fallback = ehretval_t::make_ ## ehtype(fallback_value); \
  ehretval_p out = call_method(in, #method_name, 0, NULL, context); \
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
    ehretval_p out = call_method(in, "toArray", 0, NULL, context);
    if(out->type() == array_e) {
      return out;
    } else {
      eh_error_type("toArray does not return an array", out->type(), enotice_e);
      eharray_t *arr = new eharray_t;
      arr->int_indices[0] = in;
      return this->make_array(arr);
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
