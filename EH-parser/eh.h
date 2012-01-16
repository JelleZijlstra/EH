/*
 * eh.h
 * Jelle Zijlstra, December 2011
 *
 * Main header file for the EH scripting language
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <exception>
#include <iostream>

/*
 * The EH AST
 */

/*
 * Enums used in the parser and interpreter
 */ 
// magic variables
typedef enum magicvar_enum {
	this_e,
} magicvar_enum;

typedef enum type_enum {
	null_e = 0,
	string_e = 1,
	int_e = 2,
	bool_e = 3,
	accessor_e = 4, // for internal use with array/object accessors
	type_e = 5, // for internal use with type casting
	array_e,
	func_e, // methods
	reference_e, // for internal use with lvalues, and as a value for references
	creference_e, // constant references: can be dereferenced but not written to
	object_e,
	magicvar_e,
	op_e,
	attribute_e,
	attributestr_e,
	range_e,
} type_enum;

// attributes of class members
typedef enum visibility_enum {
	public_e = 0,
	private_e = 1,
} visibility_enum;

typedef enum static_enum {
	nonstatic_e = 0,
	static_e = 1,
} static_enum;

typedef enum const_enum {
	nonconst_e = 0,
	const_e = 1,
} const_enum;

// struct for class member attributes
typedef struct memberattribute_t {
	visibility_enum visibility : 2;
	static_enum isstatic : 1;
	const_enum isconst : 1;
} memberattribute_t;

// and accompanying enum used by the parser
typedef enum attribute_enum {
	publica_e,
	privatea_e,
	statica_e,
	consta_e
} attribute_enum;

typedef enum functype_enum {
	user_e = 0,
	lib_e = 1,
} functype_enum;

typedef enum accessor_enum {
	arrow_e,
	dot_e,
	doublecolon_e
} accessor_enum;

/*
 * Parser and interpreter structs
 */
// Operator
typedef struct opnode_t {
	int op; // Type of operator
	int nparas; // Number of parameters
	struct ehretval_t **paras; // Parameters
} opnode_t;

// EH variable, and generic node
typedef struct ehretval_t {
	type_enum type;
	union {
		// simple EH variable type
		int intval;
		char *stringval;
		bool boolval;
		// complex types
		struct ehvar_t **arrayval;
		struct ehobj_t *objectval;
		struct ehretval_t *referenceval;
		struct ehfm_t *funcval;
		// pseudo-types for internal use
		opnode_t *opval;
		type_enum typeval;
		attribute_enum attributeval;
		memberattribute_t attributestrval;
		accessor_enum accessorval;
		magicvar_enum magicvarval;
		int *rangeval;
	};
} ehretval_t;

typedef struct ehvar_t {
	// union: either the name of a variable or a numeric array index
	union {
		char *name;
		int index;
	};
	// either the scope of a variable or the index-type of an array member
	union {
		int scope;
		type_enum indextype;
	};
	struct ehretval_t value;
	struct ehvar_t *next;
} ehvar_t;

// in future, add type for type checking
typedef struct eharg_t {
	char *name;
} eharg_t;

// EH object
typedef struct ehobj_t {
	char *classname;
	struct ehclassmember_t **members;
} ehobj_t;

// context
typedef ehobj_t *ehcontext_t;

// class
typedef struct ehclass_t {
	ehobj_t obj;
	struct ehclass_t *next;
} ehclass_t;

// struct with common infrastructure for procedures and methods
typedef struct ehfm_t {
	functype_enum type;
	int argcount;
	eharg_t *args;
	union {
		ehretval_t *code;
		void (*ptr)(ehretval_t *, ehretval_t *, ehcontext_t);
	};
} ehfm_t;

// EH procedure
typedef struct ehfunc_t {
	const char *name;
	struct ehfm_t f;
	struct ehfunc_t *next;
} ehfunc_t;

// Properties and methods of a class
typedef struct ehclassmember_t {
	memberattribute_t attribute;
	char *name;
	ehretval_t value;
	struct ehclassmember_t *next;
} ehclassmember_t;

typedef struct ehlibfunc_t {
	void (*code)(ehretval_t *, ehretval_t *, ehcontext_t);
	const char *name;
} ehlibfunc_t;

/*
 * EH error system
 */
typedef enum {
	efatal_e,
	eparsing_e,
	eerror_e, // runtime non-fatal error
	enotice_e,
} errlevel_e;

void eh_error(const char *message, errlevel_e level);
void eh_error_type(const char *context, type_enum type, errlevel_e level);
void eh_error_looplevels(const char *context, int levels);
void eh_error_unknown(const char *kind, const char *name, errlevel_e level);
void eh_error_redefine(const char *kind, const char *name, errlevel_e level);
void eh_error_int(const char *message, int opcode, errlevel_e level);
void eh_error_argcount(int expected, int received);
void eh_error_line(int line, const char *msg);
void eh_error_types(int context, type_enum type1, type_enum type2, errlevel_e level);
void eh_error_argcount_lib(const char *name, int expected, int received);
/*
 * Other global functions
 */
const char *get_typestring(type_enum type);


/*
 * Macros for type checking
 */
#define EH_IS_INT(var) (var.type == int_e)
#define EH_IS_STRING(var) (var.type == string_e)
#define EH_IS_ARRAY(var) (var.type == array_e)
#define EH_IS_BOOL(var) (var.type == bool_e)
#define EH_IS_NULL(var) (var.type == null_e)

/*
 * Top-level functions
 */
void eh_init(void);
void eh_exit(void);
void yyerror(void *, const char *s);
void *Malloc(size_t size);
void *Calloc(size_t count, size_t size);
void free_node(ehretval_t *in);
ehretval_t *eh_addnode(int operations, int noperations, ...);
ehretval_t eh_execute(ehretval_t *node, ehcontext_t context);
void print_tree(ehretval_t *in, int n);
void eh_setarg(int argc, char **argv);

// eh_get_x functions
#define GETFUNCPROTO(name, vtype) ehretval_t *eh_get_ ## name(vtype value);
ehretval_t *eh_get_null(void);
GETFUNCPROTO(int, int)
GETFUNCPROTO(string, char *)
GETFUNCPROTO(accessor, accessor_enum)
GETFUNCPROTO(type, type_enum)
GETFUNCPROTO(bool, bool)
GETFUNCPROTO(visibility, visibility_enum)
GETFUNCPROTO(magicvar, magicvar_enum)
GETFUNCPROTO(attribute, attribute_enum)

// indicate that we're returning
extern bool returning;

char *eh_getinput(void);
void eh_interactive(void);
extern bool is_interactive;

/*
 * EH interpreter
 */
#include "eh.bison.hpp"
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
ehclassmember_t *class_insert_retval(ehclassmember_t **classarr, char *name, memberattribute_t attribute, ehretval_t value);
ehclassmember_t *class_getmember(ehobj_t *classobj, char *name, ehcontext_t context);
ehretval_t class_get(ehobj_t *classobj, char *name, ehcontext_t context);
ehretval_t object_access(ehretval_t name, ehretval_t *index, ehcontext_t context, int token);
ehretval_t colon_access(ehretval_t operand1, ehretval_t *index, ehcontext_t context, int token);
bool ehcontext_compare(ehcontext_t lock, ehcontext_t key);


// generic initval for the hash function if no scope is applicable (i.e., for functions, which are not currently scoped)
#define HASH_INITVAL 234092
unsigned int hash(const char *data, int scope);

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

/*
 * The EH parser
 */
struct EHParser {
public:
	void *scanner;
	struct yy_buffer_state *buffer;
	ehretval_t parse_file(FILE *infile);
	ehretval_t parse_string(char *cmd);
	EHParser(void);
	~EHParser(void);
};
// put this at the bottom because of dependencies
#include "ehi.h"
