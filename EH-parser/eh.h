/*
 * eh.h
 * Jelle Zijlstra, December 2011–May 2012
 *
 * Main header file for the EH scripting language
 */
#ifndef EH_H_
#define EH_H_

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <exception>
#include <iostream>
#include <map>
#include <algorithm>
#include <list>

#include "refcount_ptr.h"
#include "eh_gc.h"

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
	object_e,
	weak_object_e,
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
typedef struct attributes_t {
	visibility_enum visibility : 2;
	static_enum isstatic : 1;
	const_enum isconst : 1;
	
	// can't make a constructor because this thing appears in a union, but this
	// is almost as good
	static attributes_t make(visibility_enum v, static_enum s, const_enum c) {
		attributes_t out;
		out.visibility = v;
		out.isstatic = s;
		out.isconst = c;
		return out;
	}
} attributes_t;

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

#include "eh_types.h"

/*
 * Parser and interpreter structs
 */

// Operator
typedef struct opnode_t {
	int op; // Type of operator
	int nparas; // Number of parameters
	ehretval_p *paras; // Parameters
	
	~opnode_t() {
		if(paras != NULL) {
			delete[] paras;
		}
	}
} opnode_t;

typedef struct ehlm_listentry_t {
	const char *name;
	ehlibmethod_t func;
} ehlm_listentry_t;

typedef struct ehlibclass_t {
	ehconstructor_t constructor;
	ehdestructor_t destructor;
	ehlm_listentry_t *members;
} ehlibclass_t;

// function executing a command
typedef ehretval_p (*ehcmd_t)(eharray_t *paras);

/*
 * Other global functions
 */
const char *get_typestring(type_enum type);
int eh_outer_exit(int exitval);
void yyerror(void *, const char *s);

/*
 * EH interpreter
 */
#include "eh.bison.hpp"
#include "ehi.h"

ehretval_p int_arrow_get(ehretval_p operand1, ehretval_p operand2);
ehretval_p string_arrow_get(ehretval_p operand1, ehretval_p operand2);
ehretval_p range_arrow_get(ehretval_p operand1, ehretval_p operand2);
void int_arrow_set(ehretval_p input, ehretval_p index, ehretval_p rvalue);
void string_arrow_set(ehretval_p input, ehretval_p index, ehretval_p rvalue);
void range_arrow_set(ehretval_p input, ehretval_p index, ehretval_p rvalue);
ehretval_p eh_count(const ehretval_p in);
ehretval_p eh_op_tilde(ehretval_p in);
ehretval_p eh_op_uminus(ehretval_p in);
ehretval_p eh_op_dot(ehretval_p operand1, ehretval_p operand2);
ehretval_p eh_make_range(const int min, const int max);

// type casting
ehretval_p eh_stringtoint(const char *const in);
ehretval_p eh_stringtofloat(const char *const in);
ehretval_p eh_stringtorange(const char *const in);
char *eh_inttostring(const int in);
ehretval_p eh_xtoint(ehretval_p in);
ehretval_p eh_xtofloat(ehretval_p in);
ehretval_p eh_xtostring(ehretval_p in);
bool eh_xtobool(ehretval_p in);
ehretval_p eh_xtorange(ehretval_p in);
ehretval_p eh_looseequals(ehretval_p operand1, ehretval_p operand2);
bool eh_strictequals(ehretval_p operand1, ehretval_p operand2);

#endif /* EH_H_ */
