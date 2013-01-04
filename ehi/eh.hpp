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
#include <typeindex>
#include <unordered_set>

#include "refcount_ptr.hpp"
#include "eh_gc.hpp"

template<class T>
static void add_tabs(T &out, int levels) {
	for(int i = 0; i < levels; i++) {
		out << "\t";
	}
}

/*
 * Enums used in the parser and interpreter
 */

// attributes of class members
enum visibility_enum {
	public_e = 0,
	private_e = 1
};

enum static_enum {
	nonstatic_e = 0,
	static_e = 1
};

enum const_enum {
	nonconst_e = 0,
	const_e = 1
};

// struct for class member attributes
struct attributes_t {
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
	static attributes_t make_private() {
		return make(private_e, nonstatic_e, nonconst_e);
	}
};

#include "eh_types.hpp"

/*
 * Other global functions
 */
void yyerror(void *, const char *s);

/*
 * EH interpreter
 */
#include "ehi.hpp"

#endif /* EH_H_ */
