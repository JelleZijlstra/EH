/*
 * eh.h
 * Jelle Zijlstra, December 2011–May 2012
 *
 * Main header file for the EH scripting language
 */
#ifndef EH_H_
#define EH_H_

#include <assert.h>
#include <stdint.h>

#include <exception>
#include <iostream>
#include <map>
#include <algorithm>
#include <list>

#include "refcount_ptr.hpp"
#include "eh_gc.hpp"

/*
 * Enums used in the parser and interpreter
 */

enum type_enum {
	null_e = 0,
	string_e = 1,
	int_e = 2,
	bool_e = 3,
	type_e = 4, // for internal use with type casting
	array_e = 5,
	hash_e = 6,
	func_e = 7, // methods
	object_e = 8,
	op_e = 10,
	attribute_e = 11,
	attributestr_e = 12,
	range_e = 13,
	float_e = 14,
	resource_e = 15,
	binding_e = 16,
	super_class_e = 17,
	tuple_e = 18
};

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
	static attributes_t make(visibility_enum v = public_e, static_enum s = nonstatic_e, const_enum c = nonconst_e) {
		attributes_t out;
		out.visibility = v;
		out.isstatic = s;
		out.isconst = c;
		return out;
	}
	// convenience methods
	static attributes_t make_const() {
		return make(public_e, nonstatic_e, const_e);
	}
	static attributes_t make_static() {
		return make(public_e, static_e, nonconst_e);
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
	user_e,
	lib_e
} functype_enum;

#include "eh_types.hpp"

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

	static opnode_t *make(int op, int nparas) {
		opnode_t *out = new opnode_t;
		out->op = op;
		out->nparas = nparas;
		if(nparas > 0) {
			out->paras = new ehretval_p[nparas];
		}
		return out;
	}
} opnode_t;

/*
 * Other global functions
 */
const char *get_typestring(type_enum type);
void yyerror(void *, const char *s);

/*
 * EH interpreter
 */
#include "eh.bison.hpp"
#include "ehi.hpp"

#endif /* EH_H_ */
