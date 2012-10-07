/*
 * Function class
 */
#ifndef EH_FUNCTION_H_
#define EH_FUNCTION_H_

#include "std_lib_includes.h"

// structure for function arguments
class eharg_t {
public:
	// in future, add type for type checking
	std::string name;

	eharg_t() : name() {}
};

/*
 * EH functions. Unlike other primitive types, functions must always be wrapped
 * in objects in order to preserve scope.
 */
class ehfunc_t {
public:
	functype_enum type;
	ehretval_p args;
	ehretval_p code;
	ehlibmethod_t libmethod_pointer;

	ehfunc_t(functype_enum _type = user_e) : type(_type), args(), code(), libmethod_pointer(NULL) {}

	~ehfunc_t() {}
private:
	ehfunc_t(const ehfunc_t&);
	ehfunc_t operator=(const ehfunc_t&);
};

// method binding
class ehbinding_t {
public:
	ehretval_p object_data;
	ehretval_p method;

	ehbinding_t(ehretval_p _object_data, ehretval_p _method) : object_data(_object_data),  method(_method) {}
};

EH_METHOD(Function, operator_colon);
EH_METHOD(Function, toString);
EH_METHOD(Function, decompile);

EXTERN_EHLC(Function)

#endif /* EH_FUNCTION_H_ */
