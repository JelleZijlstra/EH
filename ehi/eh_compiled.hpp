// Functionality to ease the duties of the EH compiler

#include "eh.hpp"
#include "eh_files.hpp"
#include "std_lib/Binding.hpp"
#include "std_lib/ConstError.hpp"
#include "std_lib/Enum.hpp"
#include "std_lib/Exception.hpp"
#include "std_lib/Function.hpp"
#include "std_lib/MiscellaneousError.hpp"
#include "std_lib/NameError.hpp"
#include "std_lib/Range.hpp"
#include "std_lib/SuperClass.hpp"
#include "std_lib/TypeError.hpp"

static const char *get_filename();
ehval_p eh_main(EHI *ehi, const ehcontext_t &context);

namespace eh_compiled {

typedef void (*class_f)(const ehcontext_t &, EHI *);
typedef void (*finally_f)(const ehcontext_t &, EHI *);

static inline ehval_p make_closure(Function::compiled_method method, const ehcontext_t &context, EHI *ehi) {
	ehval_p function_object = ehi->get_parent()->function_object;
	ehobj_t *function_obj = new ehobj_t();
	ehval_p func = Object::make(function_obj, ehi->get_parent());
	function_obj->type_id = function_object->get<Object>()->type_id;
	Function::t *f = new Function::t(Function::compiled_e);
	f->compiled_pointer = method;
	function_obj->object_data = Function::make(f, ehi->get_parent());
	function_obj->inherit(function_object);
	function_obj->parent = context.scope;
	return func;
}

static inline ehval_p get_variable(const char *name, const ehcontext_t &context, EHI *ehi) {
	ehmember_p var = context.scope->get<Object>()->get_recursive(name, context);
	if(var == nullptr) {
		throw_NameError(context.scope, name, ehi);
		return nullptr;
	} else {
		return var->value;
	}
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

static inline ehval_p make_class(const char *name, class_f code, const ehcontext_t &context, EHI *ehi) {
	EHInterpreter *parent = ehi->get_parent();
	ehobj_t *new_obj = new ehobj_t();
	ehval_p ret = Object::make(new_obj, parent);

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

};

int main(int argc, char *argv[]) {
	EHInterpreter interpreter;
	interpreter.eh_setarg(argc + 1, argv - 1);
	EHI ehi(end_is_end_e, &interpreter, interpreter.global_object, eh_getcwd(), get_filename());
	try {
		eh_main(&ehi, ehi.get_context());
	} catch(eh_exception &e) {
		ehi.handle_uncaught(e);
		return 1;
	} catch(quit_exception &) {
		return 1;
	}
	return 0;
}
