/*
 * eh_libclasses.h
 *
 * Header file for EH library classes
 */
#ifndef _EH_LIBCLASSES_H
#define _EH_LIBCLASSES_H

#include <exception>

#define EH_INITIALIZER(name) void ehinit_ ## name (ehclass_t *cls, EHInterpreter *parent)
#define EH_INSTANCE_INITIALIZER(name) void ehinit_ ## name (ehclass_t *obj, EHInterpreter *parent)
#define REGISTER_METHOD(classn, name) cls->register_method(#name, &ehlm_ ## classn ## _ ## name, attributes_t(), parent)
#define REGISTER_METHOD_RENAME(classn, name, user_name) cls->register_method(user_name, &ehlm_ ## classn ## _ ## name, attributes_t(), parent)
#define REGISTER_STATIC_METHOD(classn, name) cls->register_static_method(#name, &ehlm_ ## classn ## _ ## name, attributes_t(), parent)
#define REGISTER_STATIC_METHOD_RENAME(classn, name, user_name) cls->register_static_method(user_name, &ehlm_ ## classn ## _ ## name, attributes_t(), parent)
#define REGISTER_CONSTRUCTOR(classn) REGISTER_STATIC_METHOD_RENAME(classn, operator_colon, "operator()")
#define REGISTER_CLASS(classn, name) cls->register_member_class<classn ## _ ## name>(ehinit_ ## classn ##_ ## name, #name, attributes_t(), parent)
#define REGISTER_CONSTANT(classn, name, value) cls->register_value(#name, value, attributes_t(public_e, static_e, const_e))
#define INHERIT_LIBRARY(classname) 	cls->inherit(parent->repo.get_primitive_class<classname>())

#define EH_METHOD(classn, name) ehval_p ehlm_ ## classn ## _ ## name(ehval_p obj, ehval_p args, EHI *ehi)

#define ASSERT_TYPE(operand, ehtype, method) do {\
	operand->assert_type<ehtype>(method, ehi); \
} while(0)

#define ASSERT_NARGS(count, method) args->assert_type<Tuple>(method, ehi); \
	if(args->get<Tuple>()->size() != count) { \
		throw_ArgumentError("Argument must be a tuple of size " #count, method, args, ehi); \
	}

#define ASSERT_OBJ_TYPE(ehtype, method) ehval_p _obj = obj; \
if(!obj->is_a<ehtype>()) { \
	throw_TypeError("Invalid base object for " #method, obj, ehi); \
}

#define ASSERT_NULL(method) if(!args->is_a<Null>()) { \
	throw_TypeError("Argument to " #method " must be null", args, ehi); \
}

#define ASSERT_NARGS_AND_TYPE(count, ehtype, method) ASSERT_NARGS(count, method); ASSERT_OBJ_TYPE(ehtype, method);
#define ASSERT_NULL_AND_TYPE(ehtype, method) ASSERT_NULL(method); ASSERT_OBJ_TYPE(ehtype, method);

#define ASSERT_RESOURCE(ehtype, method) ASSERT_OBJ_TYPE(ehtype, method); \
	ehtype::type data = obj->get<ehtype>();

#define EH_CHILD_CLASS(parent, cname) class parent ## _ ## cname; \
	template<> inline const char *ehval_t::name<parent ## _ ## cname>() { return #parent "." #cname; } \
	class parent ## _ ## cname : public ehval_t

// has to come after macro definitions for EH_METHOD and EXTERN_EHLC

// Helpers
template<class T>
static inline int intcmp(T lhs, T rhs) {
	if(lhs < rhs) {
		return -1;
	} else if(lhs == rhs) {
		return 0;
	} else {
		return 1;
	}
}

class quit_exception : public std::exception {
};

#ifdef __clang__
#define EH_NORETURN [[noreturn]]
#else
#define EH_NORETURN
#endif

#endif /* _EH_LIBCLASSES_H */
