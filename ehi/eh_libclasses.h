/*
 * eh_libclasses.h
 *
 * Header file for EH library classes
 */
#ifndef _EH_LIBCLASSES_H
#define _EH_LIBCLASSES_H

#include "eh.h"

#define EH_INITIALIZER(name) void ehinit_ ## name (ehobj_t *obj, EHInterpreter *parent)
#define REGISTER_METHOD(classn, name) obj->register_method(#name, &ehlm_ ## classn ## _ ## name, attributes_t::make(), parent)
#define REGISTER_METHOD_RENAME(classn, name, user_name) obj->register_method(user_name, &ehlm_ ## classn ## _ ## name, attributes_t::make(), parent)
#define REGISTER_CLASS(classn, name) obj->register_member_class(#name, -1, ehinit_ ## classn ##_ ## name, attributes_t::make(), parent)
#define REGISTER_CONSTANT(classn, name, value) obj->register_value(#name, value, attributes_t::make(public_e, static_e, const_e))
#define INHERIT_LIBRARY(classname) 	obj->inherit(parent->global_object->get_objectval()->get_known(#classname)->value)

#define EH_METHOD(classn, name) ehretval_p ehlm_ ## classn ## _ ## name(ehretval_p obj, ehretval_p args, EHI *ehi)

#define ASSERT_TYPE(operand, ehtype, method) if(!operand->is_a(ehtype)) { \
	throw_TypeError("Invalid type for argument to " method, operand->type(), ehi); \
}

#define ASSERT_NARGS(count, method) ASSERT_TYPE(args, tuple_e, method); \
	if(args->get_tupleval()->size() != count) { \
		throw_ArgumentError("Argument must be a tuple of size " #count, method, args, ehi); \
	}

#define ASSERT_OBJ_TYPE(ehtype, method) ehretval_p _obj = obj; \
obj = ehretval_t::self_or_data(obj); \
if(!obj->is_a(ehtype)) { \
	throw_TypeError("Invalid base object for " #method, obj->type(), ehi); \
}

#define ASSERT_NULL(method) if(args->type() != null_e) { \
	throw_TypeError("Argument to " #method " must be null", args->type(), ehi); \
}

#define ASSERT_NARGS_AND_TYPE(count, ehtype, method) ASSERT_NARGS(count, method); ASSERT_OBJ_TYPE(ehtype, method);
#define ASSERT_NULL_AND_TYPE(ehtype, method) ASSERT_NULL(method); ASSERT_OBJ_TYPE(ehtype, method);

class LibraryBaseClass {
public:
	virtual ~LibraryBaseClass() {}
};

// has to come after macro definitions for EH_METHOD and EXTERN_EHLC, as well
// as definition of LibraryBaseClass
#include "std_lib/TypeError.h"
#include "std_lib/ArgumentError.h"
#include "std_lib/EmptyIterator.h"
#include "std_lib/Tuple.h"
#include "std_lib/Range.h"
#include "std_lib/Hash.h"
#include "std_lib/Array.h"

// Helpers
static inline int intcmp(int lhs, int rhs) {
	if(lhs < rhs) {
		return -1;
	} else if(lhs == rhs) {
		return 0;
	} else {
		return 1;
	}	
}

#endif /* _EH_LIBCLASSES_H */
