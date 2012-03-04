/*
 * eh.h
 * Jelle Zijlstra, December 2011
 *
 * Main header file for the EH scripting language
 */
#ifndef EH_H_
#define EH_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <exception>
#include <iostream>

#include "eh_gc.h"

/*
 * The EH AST
 */

/*
 * Enums used in the parser and interpreter
 */

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
	op_e,
	attribute_e,
	attributestr_e,
	range_e,
	float_e
} type_enum;

// attributes of class members
typedef enum visibility_enum {
	public_e = 0,
	private_e = 1
} visibility_enum;

typedef enum static_enum {
	nonstatic_e = 0,
	static_e = 1
} static_enum;

typedef enum const_enum {
	nonconst_e = 0,
	const_e = 1
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
	libmethod_e = 2
} functype_enum;

typedef enum accessor_enum {
	arrow_e,
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
		float floatval;
		// complex types
		struct ehvar_t **arrayval;
		struct ehobj_t *objectval;
		struct ehretval_t *referenceval;
		struct ehfm_t *funcval;
		struct ehrange_t *rangeval;
		// pseudo-types for internal use
		opnode_t *opval;
		type_enum typeval;
		attribute_enum attributeval;
		memberattribute_t attributestrval;
		accessor_enum accessorval;
	};
	ehretval_t() {
		refcount = 0;
	}
	void inc_rc() {
		refcount++;
	}
	void dec_rc() {
		refcount--;
		if(refcount == 0) {
			free();
		}
	}
	void free();
private:
	short refcount;
} ehretval_t;

typedef struct ehvar_t {
	// union: either the name of a variable or a numeric array index
	union {
		const char *name;
		int index;
	};
	// the scope of a variable, the index-type of an array member, or the
	// attributes of an object member
	union {
		memberattribute_t attribute;
		struct ehscope_t *scope;
		type_enum indextype;
	};
	struct ehretval_t *value;
	struct ehvar_t *next;
} ehvar_t;

// in future, add type for type checking
typedef struct eharg_t {
	const char *name;
} eharg_t;

// context
typedef const struct ehobj_t *ehcontext_t;

typedef void *(*ehconstructor_t)();

typedef void (*ehlibmethod_t)(void *, ehretval_t *, ehretval_t *, ehcontext_t);

typedef struct ehlibentry_t {
	const char *name;
	ehlibmethod_t func;
} ehlibentry_t;

typedef struct ehlibclass_t {
	ehconstructor_t constructor;
	ehlibentry_t *members;
} ehlibclass_t;

typedef struct ehlc_listentry_t {
	const char *name;
	ehlibclass_t info;
} ehlc_listentry_t;

// EH object
typedef struct ehobj_t {
	const char *classname;
	void *selfptr;
	union {
		struct ehvar_t **members;
		ehlibclass_t libinfo;
	};
} ehobj_t;

// range
typedef struct ehrange_t {
	int min;
	int max;
} ehrange_t;

// function executing a command
typedef ehretval_t (*ehcmd_f_t)(ehvar_t **paras);

// command
typedef struct ehcmd_t {
	const char *name;
	ehcmd_f_t code;
} ehcmd_t;

typedef struct ehcmd_bucket_t {
	ehcmd_t cmd;
	struct ehcmd_bucket_t *next;
} ehcmd_bucket_t;

// class
typedef struct ehclass_t {
	ehobj_t obj;
	functype_enum type;
	struct ehclass_t *next;
} ehclass_t;

// struct with common infrastructure for procedures and methods
typedef struct ehfm_t {
	functype_enum type;
	int argcount;
	eharg_t *args;
	union {
		const ehretval_t *code;
		void (*ptr)(ehretval_t *, ehretval_t *, ehcontext_t);
		ehlibmethod_t mptr;
	};
} ehfm_t;

// EH procedure
typedef struct ehlibfunc_t {
	void (*code)(ehretval_t *, ehretval_t *, ehcontext_t);
	const char *name;
} ehlibfunc_t;

// scope
struct ehscope_t {
	struct ehscope_t *parent;
};

/*
 * EH error system
 */
typedef enum {
	efatal_e,
	eparsing_e,
	eerror_e, // runtime non-fatal error
	enotice_e
} errlevel_e;

void eh_error(const char *message, errlevel_e level);
void eh_error_type(const char *context, type_enum type, errlevel_e level);
void eh_error_looplevels(const char *context, int levels);
void eh_error_unknown(const char *kind, const char *name, errlevel_e level);
void eh_error_redefine(const char *kind, const char *name, errlevel_e level);
void eh_error_int(const char *message, int opcode, errlevel_e level);
void eh_error_argcount(int expected, int received);
void eh_error_line(int line, const char *msg);
void eh_error_types(const char *context, type_enum type1, type_enum type2, errlevel_e level);
void eh_error_argcount_lib(const char *name, int expected, int received);
/*
 * Other global functions
 */
const char *get_typestring(type_enum type);
int eh_outer_exit(int exitval);

/*
 * Top-level functions
 */
void eh_init(void);
void eh_exit(void);
void yyerror(void *, const char *s);
void free_node(ehretval_t *in);
ehretval_t *eh_addnode(int operations, int noperations, ...);
ehretval_t eh_execute(const ehretval_t *node, const ehcontext_t context);
void print_tree(const ehretval_t *const in, const int n);
void eh_setarg(int argc, char **argv);

// eh_get_x functions
#define GETFUNCPROTO(name, vtype) ehretval_t *eh_get_ ## name(vtype value);
ehretval_t *eh_get_null(void);
GETFUNCPROTO(int, int)
GETFUNCPROTO(string, char *)
GETFUNCPROTO(float, float)
GETFUNCPROTO(accessor, accessor_enum)
GETFUNCPROTO(type, type_enum)
GETFUNCPROTO(bool, bool)
GETFUNCPROTO(visibility, visibility_enum)
GETFUNCPROTO(attribute, attribute_enum)

// indicate that we're returning
extern bool returning;

char *eh_getinput(void);

// represents the interactivity of the parser. 0 is non-interactive, 1 just reads stuff from stdin, 2 gives a prompt. Of course, this should really be a property of the interpreter object.
extern int is_interactive;

/*
 * EH interpreter
 */
#include "eh.bison.hpp"
// symbol table for variables and functions
#define VARTABLE_S 1024
extern ehvar_t *vartable[];
extern ehclass_t *classtable[];
extern ehcmd_bucket_t *cmdtable[];

// current variable scope
extern ehscope_t *global_scope;
extern ehscope_t *curr_scope;

// prototypes
bool insert_variable(ehvar_t *var);
ehvar_t *get_variable(const char *name, ehscope_t *scope, ehcontext_t context);
void remove_variable(const char *name, ehscope_t *scope);
void list_variables(void);
ehretval_t call_function(const ehfm_t *f, ehretval_t *args, ehcontext_t context, ehcontext_t newcontext);
ehretval_t call_function_args(const ehfm_t *const f, const ehcontext_t context, const ehcontext_t newcontext, const int nargs, const ehretval_t *const args);
void array_insert(ehvar_t **array, ehretval_t *in, int place, ehcontext_t context);
ehvar_t *array_insert_retval(ehvar_t **array, ehretval_t index, ehretval_t ret);
ehvar_t *array_getmember(ehvar_t **array, ehretval_t index);
ehretval_t array_get(ehvar_t **array, ehretval_t index);
int array_count(ehvar_t **array);
void insert_class(ehclass_t *classobj);
ehclass_t *get_class(const char *name);
void class_copy_member(ehobj_t *classobj, ehvar_t *classmember, int i);
void class_insert(ehvar_t **classarr, const ehretval_t *in, ehcontext_t context);
ehvar_t *class_insert_retval(ehvar_t **classarr, const char *name, memberattribute_t attribute, ehretval_t value);
ehvar_t *class_getmember(const ehobj_t *classobj, const char *name, ehcontext_t context);
ehretval_t class_get(const ehobj_t *classobj, const char *name, ehcontext_t context);
ehretval_t object_access(ehretval_t name, const ehretval_t *index, ehcontext_t context, int token);
ehretval_t colon_access(ehretval_t operand1, const ehretval_t *index, ehcontext_t context, int token);
bool ehcontext_compare(const ehcontext_t lock, const ehcontext_t key);


// generic initval for the hash function if no scope is applicable (i.e., for functions, which are not currently scoped)
#define HASH_INITVAL 234092
unsigned int hash(const char *data, int scope);

// type casting
ehretval_t eh_cast(const type_enum type, const ehretval_t in);
ehretval_t eh_stringtoint(const char *const in);
ehretval_t eh_stringtofloat(const char *const in);
ehretval_t eh_stringtorange(const char *const in);
ehretval_t eh_rangetoarray(const ehrange_t *const range);
char *eh_inttostring(const int in);
ehretval_t eh_xtoarray(const ehretval_t in);
ehretval_t eh_xtoint(const ehretval_t in);
ehretval_t eh_xtofloat(const ehretval_t in);
ehretval_t eh_xtostring(const ehretval_t in);
ehretval_t eh_xtobool(const ehretval_t in);
ehretval_t eh_xtorange(const ehretval_t in);
ehretval_t eh_looseequals(ehretval_t operand1, ehretval_t operand2);
ehretval_t eh_strictequals(ehretval_t operand1, ehretval_t operand2);

/*
 * Helper
 */
int eh_getargs(ehretval_t *paras, int n, ehretval_t *args, ehcontext_t context, const char *name);
void print_retval(const ehretval_t in);

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
#endif /* EH_H_ */
