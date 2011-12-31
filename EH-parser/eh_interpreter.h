/*
 * eh_interpreter.h
 * Jelle Zijlstra, December 2011
 *
 * Header file for the EH interpreter
 */
#include "eh_error.h"
#include "y.tab.h"
// symbol table for variables and functions
#define VARTABLE_S 1024
extern ehvar_t *vartable[];
extern ehfunc_t *functable[];
extern ehclass_t *classtable[];

// current variable scope
extern int scope;

// prototypes
bool insert_variable(ehvar_t *var);
ehvar_t *get_variable(char *name, int scope);
void remove_variable(char *name, int scope);
void list_variables(void);
bool insert_function(ehfunc_t *func);
ehfunc_t *get_function(char *name);
ehretval_t call_function(ehfm_t *f, ehretval_t *args, ehcontext_t context, ehcontext_t newcontext);
void array_insert(ehvar_t **array, ehretval_t *in, int place, ehcontext_t context);
ehvar_t *array_insert_retval(ehvar_t **array, ehretval_t index, ehretval_t ret);
ehvar_t *array_getmember(ehvar_t **array, ehretval_t index);
ehretval_t array_get(ehvar_t **array, ehretval_t index);
int array_count(ehvar_t **array);
void insert_class(ehclass_t *classobj);
ehclass_t *get_class(char *name);
void class_insert(ehclassmember_t **classarr, ehretval_t *in, ehcontext_t context);
ehclassmember_t *class_getmember(ehobj_t *classobj, char *name, ehcontext_t context);
ehretval_t class_get(ehobj_t *classobj, char *name, ehcontext_t context);
ehretval_t object_access(ehretval_t name, ehretval_t *index, ehcontext_t context);
ehretval_t colon_access(ehretval_t operand1, ehretval_t *index, ehcontext_t context);
bool ehcontext_compare(ehcontext_t lock, ehcontext_t key);
// commands
void execute_command(char *, ehvar_t **);


// generic initval for the hash function if no scope is applicable (i.e., for functions, which are not currently scoped)
#define HASH_INITVAL 234092
unsigned int hash(char *data, int scope);

// type casting
ehretval_t eh_strtoi(char *in);
char *eh_itostr(int in);
ehretval_t eh_xtoi(ehretval_t in);
ehretval_t eh_xtostr(ehretval_t in);
ehretval_t eh_xtobool(ehretval_t in);
ehretval_t eh_looseequals(ehretval_t operand1, ehretval_t operand2);
ehretval_t eh_strictequals(ehretval_t operand1, ehretval_t operand2);

/*
 * macros for interpreter behavior
 */
// take ints, returns an int
#define EH_INT_CASE(token, operator) case token: \
	operand1 = eh_xtoi(eh_execute(node->opval->paras[0], context)); \
	operand2 = eh_xtoi(eh_execute(node->opval->paras[1], context)); \
	if(EH_IS_INT(operand1) && EH_IS_INT(operand2)) { \
		ret.type = int_e; \
		ret.intval = (operand1.intval operator operand2.intval); \
	} \
	else \
		eh_error_types(token, operand1.type, operand2.type, eerror_e); \
	break;
// take ints, return a bool
#define EH_INTBOOL_CASE(token, operator) case token: \
	operand1 = eh_xtoi(eh_execute(node->opval->paras[0], context)); \
	operand2 = eh_xtoi(eh_execute(node->opval->paras[1], context)); \
	if(EH_IS_INT(operand1) && EH_IS_INT(operand2)) { \
		ret.type = bool_e; \
		ret.boolval = (operand1.intval operator operand2.intval); \
	} \
	else { \
		eh_error_types(token, operand1.type, operand2.type, eerror_e); \
	} \
	break;
// take bools, return a bool
#define EH_BOOL_CASE(token, operator) case token: \
	operand1 = eh_xtobool(eh_execute(node->opval->paras[0], context)); \
	operand2 = eh_xtobool(eh_execute(node->opval->paras[1], context)); \
	ret.type = bool_e; \
	ret.boolval = (operand1.boolval operator operand2.boolval); \
	break;

/*
 * Stuff to be done in a loop
 */
#define LOOPCHECKS { \
	if(returning) break; \
	if(breaking) { \
		breaking--; \
		break; \
	} \
	if(continuing > 1) { \
		continuing--; \
		break; \
	} \
	else if(continuing) { \
		continuing = 0; \
		continue; \
	} \
	}

/*
 * Macros for converting between ehretval_t and ehvar_t
 */
#define SETRETFROMVAR(var) { ret = var->value; }

#define SETVARFROMRET(var) { var->value = ret; }
