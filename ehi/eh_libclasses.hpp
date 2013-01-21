/*
 * eh_libclasses.h
 *
 * Header file for EH library classes
 */
#ifndef _EH_LIBCLASSES_H
#define _EH_LIBCLASSES_H

#include <exception>

#define EH_INITIALIZER(name) void ehinit_ ## name (ehobj_t *obj, EHInterpreter *parent)
#define REGISTER_METHOD(classn, name) obj->register_method(#name, &ehlm_ ## classn ## _ ## name, attributes_t(), parent)
#define REGISTER_METHOD_RENAME(classn, name, user_name) obj->register_method(user_name, &ehlm_ ## classn ## _ ## name, attributes_t(), parent)
#define REGISTER_CLASS(classn, name) obj->register_member_class<classn ## _ ## name>(ehinit_ ## classn ##_ ## name, #name, attributes_t(), parent)
#define REGISTER_CONSTANT(classn, name, value) obj->register_value(#name, value, attributes_t(public_e, static_e, const_e))
#define INHERIT_LIBRARY(classname) 	obj->inherit(parent->global_object->get<Object>()->get_known(#classname)->value)

#define EH_METHOD(classn, name) ehval_p ehlm_ ## classn ## _ ## name(ehval_p obj, ehval_p args, EHI *ehi)

#define ASSERT_TYPE(operand, ehtype, method) do {\
 	operand = operand->data(); \
	operand->assert_type<ehtype>(method, ehi); \
} while(0)

#define ASSERT_NARGS(count, method) args->assert_type<Tuple>(method, ehi); \
	if(args->get<Tuple>()->size() != count) { \
		throw_ArgumentError("Argument must be a tuple of size " #count, method, args, ehi); \
	}

#define ASSERT_OBJ_TYPE(ehtype, method) ehval_p _obj = obj; \
obj = obj->data(); \
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


#endif /* _EH_LIBCLASSES_H */
