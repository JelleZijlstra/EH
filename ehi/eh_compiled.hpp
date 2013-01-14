// Functionality to ease the duties of the EH compiler

#include "eh.hpp"
#include "std_lib/SuperClass.hpp"
#include "std_lib/NameError.hpp"
#include "std_lib/ConstError.hpp"

static const char *get_filename();
ehval_p eh_main(EHI *ehi, const ehcontext_t &context);

namespace eh_compiled {

static inline ehval_p make_closure(Function::compiled_method method, const ehcontext_t &context, EHI *ehi) {
	ehval_p function_object = ehi->get_parent()->function_object;
	ehobj_t *function_obj = new ehobj_t();
	ehval_p func = Object::make(function_obj, ehi->get_parent());
	function_obj->type_id = function_object->get<Object>()->type_id;
	Function::t *f = new Function::t(Function::compiled_e);
	f->compiled_pointer = method;
	function_obj->object_data = Function::make(f);
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

};

int main(int argc, char *argv[]) {
	EHInterpreter interpreter;
	interpreter.eh_setarg(argc, argv);
	EHI ehi(end_is_end_e, &interpreter, interpreter.global_object, eh_getcwd(), get_filename());
	try {
		eh_main(&ehi, ehi.get_context());
	} catch (eh_exception &e) {
		ehi.handle_uncaught(e);
		return -1;
	}
	return 0;
}
