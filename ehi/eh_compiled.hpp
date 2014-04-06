// Functionality to ease the duties of the EH compiler

#include "eh.hpp"
#include "eh_files.hpp"
#include "std_lib/Array.hpp"
#include "std_lib/Binding.hpp"
#include "std_lib/ConstError.hpp"
#include "std_lib/Enum.hpp"
#include "std_lib/Exception.hpp"
#include "std_lib/Function.hpp"
#include "std_lib/Map.hpp"
#include "std_lib/MiscellaneousError.hpp"
#include "std_lib/NameError.hpp"
#include "std_lib/Range.hpp"
#include "std_lib/TypeError.hpp"

namespace eh_compiled {

typedef void (*class_f)(const ehcontext_t &, EHI *);
typedef void (*finally_f)(const ehcontext_t &, EHI *);

static inline ehval_p make_closure(Function::compiled_method method, const ehcontext_t &context, EHI *ehi) {
	Function::t *f = new Function::t(Function::compiled_e);
	f->compiled_pointer = method;
	f->parent = context.scope;
	return Function::make(f, ehi->get_parent());
}

static inline ehval_p make_range(ehval_p l, ehval_p r, EHI *ehi) {
	if(!l->equal_type(r)) {
		throw_TypeError("Range members must have the same type", r, ehi);
	}
	return Range::make(l, r, ehi->get_parent());
}

static inline bool boolify(ehval_p val, const ehcontext_t &context, EHI *ehi) {
	return ehi->toBool(val, context)->get<Bool>();
}

static inline ehval_p get_variable(const char *name, const ehcontext_t &context, EHI *ehi) {
	ehmember_p var = context.scope->get_property_up_scope_chain(name, context, ehi->get_parent());
	if(var.null()) {
		throw_NameError(context.scope, name, ehi);
	} else {
		return var->value;
	}
}

static inline ehval_p make_class(const char *name, class_f code, const ehcontext_t &context, EHI *ehi) {
	EHInterpreter *parent = ehi->get_parent();
	ehclass_t *new_obj = new ehclass_t(name);
	ehval_p ret = Class::make(new_obj, parent);

	new_obj->type_id = parent->repo.register_class(name, ret);
	new_obj->parent = context.scope;

	// execute the code within the class
	code(ret, ehi);

	return ret;
}

template<class T>
typename T::type call_function_typed(ehval_p func, ehval_p args, const ehcontext_t &context, EHI *ehi) {
	ehval_p ret = ehi->call_function(func, args, context);
	if(!ret->is_a<T>()) {
		std::ostringstream message;
		message << "Expected function to return a value of type ";
		message << ehval_t::name<T>();
		throw_TypeError(strdup(message.str().c_str()), ret, ehi);
	}
	return ret->get<T>();
}

}
