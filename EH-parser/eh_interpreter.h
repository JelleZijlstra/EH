/*
 * eh_interpreter.h
 * Jelle Zijlstra, December 2011
 *
 * Header file for the EH interpreter
 */
#include "eh.h"
#include "y.tab.h"
// symbol table for variables and functions
#define VARTABLE_S 1024
static ehvar_t *vartable[VARTABLE_S];
static ehfunc_t *functable[VARTABLE_S];

// indicate that we're returning
static bool returning = false;
// current variable scope
static int scope = 0;

// prototypes
static bool insert_variable(ehvar_t *var);
static ehvar_t *get_variable(char *name, int scope);
static void remove_variable(char *name, int scope);
static void list_variables(void);
static bool insert_function(ehfunc_t *func);
static ehfunc_t *get_function(char *name);
static void array_insert(ehvar_t **array, ehnode_t *in, int place);
static void array_insert_retval(ehvar_t **array, ehretval_t index, ehretval_t ret);
static ehvar_t *array_getmember(ehvar_t **array, ehretval_t index);
static ehretval_t array_get(ehvar_t **array, ehretval_t index);
static int array_count(ehvar_t **array);

// generic initval for the hash function if no scope is applicable (i.e., for functions, which are not currently scoped)
#define HASH_INITVAL 234092
static unsigned int hash(char *data, int scope);

// type casting
static ehretval_t eh_strtoi(char *in);
static char *eh_itostr(int in);
static ehretval_t eh_xtoi(ehretval_t in);
static ehretval_t eh_xtostr(ehretval_t in);
static ehretval_t eh_xtobool(ehretval_t in);

/*
 * macros for interpreter behavior
 */
// take ints, returns an int
#define EH_INT_CASE(token, operator) case token: \
	operand1 = eh_xtoi(execute(node->op.paras[0])); \
	operand2 = eh_xtoi(execute(node->op.paras[1])); \
	if(IS_INT(operand1) && IS_INT(operand2)) { \
		ret.type = int_e; \
		ret.intval = (operand1.intval operator operand2.intval); \
	} \
	else { \
		fprintf(stderr, "Incompatible operands\n"); \
	} \
	break;
// take ints, return a bool
#define EH_INTBOOL_CASE(token, operator) case token: \
	operand1 = eh_xtoi(execute(node->op.paras[0])); \
	operand2 = eh_xtoi(execute(node->op.paras[1])); \
	if(IS_INT(operand1) && IS_INT(operand2)) { \
		ret.type = bool_e; \
		ret.boolval = (operand1.intval operator operand2.intval); \
	} \
	else { \
		fprintf(stderr, "Incompatible operands\n"); \
	} \
	break;
// take bools, return a bool
#define EH_BOOL_CASE(token, operator) case token: \
	operand1 = eh_xtobool(execute(node->op.paras[0])); \
	operand2 = eh_xtobool(execute(node->op.paras[1])); \
	ret.type = bool_e; \
	ret.boolval = (operand1.intval operator operand2.intval); \
	break;

/*
 * Macros for converting between ehretval_t and ehvar_t
 */
#define SETRETFROMVAR(var) { ret.type = var->type; switch(ret.type) { \
	case int_e: ret.intval = var->intval; break; \
	case string_e: ret.strval = var->strval; break; \
	case array_e: ret.arrval = var->arrval; break; \
	case bool_e: ret.boolval = var->boolval; break; \
	case null_e: break; \
	default: fprintf(stderr, "Unsupported type\n"); break; \
} }

#define SETVARFROMRET(var) { var->type = ret.type; switch(ret.type) { \
	case int_e: var->intval = ret.intval; break; \
	case string_e: var->strval = ret.strval; break; \
	case array_e: var->arrval = ret.arrval; break; \
	case bool_e: var->boolval = ret.boolval; break; \
	case null_e: break; \
	default: fprintf(stderr, "Unsupported type\n"); break; \
} }

