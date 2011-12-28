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
ehvar_t *vartable[VARTABLE_S];
ehfunc_t *functable[VARTABLE_S];
ehclass_t *classtable[VARTABLE_S];

// current variable scope
extern int scope;

// prototypes
bool insert_variable(ehvar_t *var);
ehvar_t *get_variable(char *name, int scope);
void remove_variable(char *name, int scope);
void list_variables(void);
bool insert_function(ehfunc_t *func);
ehfunc_t *get_function(char *name);
ehretval_t call_function(ehfm_t *f, ehnode_t *args, char *context);
void array_insert(ehvar_t **array, ehnode_t *in, int place, char *context);
ehvar_t *array_insert_retval(ehvar_t **array, ehretval_t index, ehretval_t ret);
ehvar_t *array_getmember(ehvar_t **array, ehretval_t index);
ehretval_t array_get(ehvar_t **array, ehretval_t index);
int array_count(ehvar_t **array);
void insert_class(ehclass_t *class);
ehclass_t *get_class(char *name);
void class_insert(ehclassmember_t **class, ehnode_t *in, char *context);
ehclassmember_t *class_getmember(ehclassmember_t **class, char *name);
ehretval_t class_get(ehclassmember_t **class, char *name);

// generic initval for the hash function if no scope is applicable (i.e., for functions, which are not currently scoped)
#define HASH_INITVAL 234092
unsigned int hash(char *data, int scope);

// type casting
ehretval_t eh_strtoi(char *in);
char *eh_itostr(int in);
ehretval_t eh_xtoi(ehretval_t in);
ehretval_t eh_xtostr(ehretval_t in);
ehretval_t eh_xtobool(ehretval_t in);

/*
 * macros for interpreter behavior
 */
// take ints, returns an int
#define EH_INT_CASE(token, operator) case token: \
	operand1 = eh_xtoi(execute(node->op.paras[0], context)); \
	operand2 = eh_xtoi(execute(node->op.paras[1], context)); \
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
	operand1 = eh_xtoi(execute(node->op.paras[0], context)); \
	operand2 = eh_xtoi(execute(node->op.paras[1], context)); \
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
	operand1 = eh_xtobool(execute(node->op.paras[0], context)); \
	operand2 = eh_xtobool(execute(node->op.paras[1], context)); \
	ret.type = bool_e; \
	ret.boolval = (operand1.intval operator operand2.intval); \
	break;

/*
 * Macros for converting between ehretval_t and ehvar_t
 */
#define SETRETFROMVAR(var) { ret = var->value; }

#define SETVARFROMRET(var) { var->value = ret; }
