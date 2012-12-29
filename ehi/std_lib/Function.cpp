/*
 * Function
 * Represents functions. Internally, functions are in fact often represented as
 * bindings, pairs of an object (which will be the this object inside the
 * function) and the function itself. There is no Function constructor.
 */

#include <sstream>

#include "Function.hpp"

EH_INITIALIZER(Function) {
	REGISTER_METHOD_RENAME(Function, operator_colon, "operator()");
	REGISTER_METHOD(Function, toString);
	REGISTER_METHOD(Function, decompile);
	REGISTER_METHOD(Function, bindTo);
}

/*
 * @description Call a function. Overriding this method is not possible; it is
 * called directly by the engine, rather than using the normal call mechanisms.
 * @argument Arguments to the function call
 * @returns Return value of the function
 */
EH_METHOD(Function, operator_colon) {
	// This is probably the most important library method in EH. It works
	// on both Function and binding objects.
	ehretval_p base_object;
	ehretval_p function_object;
	if(obj->is_a(func_e)) {
		function_object = obj;
		base_object = ehi->global();
	} else if(obj->type() == binding_e) {
		ehbinding_t *binding = obj->get_bindingval();
		function_object = binding->method;
		base_object = binding->object_data;
	} else {
		throw_TypeError("Invalid base object for Function.operator()", obj->type(), ehi);
	}
	return ehfunc_t::exec(base_object, function_object, args, ehi);
}

ehretval_p ehfunc_t::exec(ehretval_p base_object, ehretval_p function_object, ehretval_p args, EHI *ehi) {
	ehfunc_t *f = function_object->get_objectval()->object_data->get_funcval();

	if(f->type == lib_e) {
		return f->libmethod_pointer(base_object, args, ehi);
	}
	ehretval_p newcontext = ehi->get_parent()->instantiate(function_object);
	newcontext->get_objectval()->object_data = function_object->get_objectval()->object_data;

	// set arguments
	attributes_t attributes = attributes_t::make(private_e, nonstatic_e, nonconst_e);
	ehi->set(f->args, args, &attributes, ehcontext_t(base_object, newcontext));

	// execute the function
	ehretval_p ret = ehi->eh_execute(f->code, ehcontext_t(base_object, newcontext));
	ehi->not_returning();
	return ret;
}


/*
 * @description "Decompiles" a function back into valid, executable EH code.
 * This is possible because the current interpreter works on an AST that
 * closely follows the structure of the EH code. Future optimizations or
 * alternative execution models may change the output of this function. For
 * C++ library functions, this method simply returns "native code".
 * @argument None
 * @returns String
 */
EH_METHOD(Function, decompile) {
	ehretval_p hold_obj;
	if(obj->type() == binding_e) {
		hold_obj = obj;
		obj = obj->get_bindingval()->method;
	}
	ASSERT_OBJ_TYPE(func_e, "Function.decompile");
	std::string reduction = obj->decompile(0);
	return ehretval_t::make_string(strdup(reduction.c_str()));
}

/*
 * @description Provides a string representation of a function. For C++
 * functions, this is simply "(args) => (native code)"; for user functions,
 * the argument list is decompiled.
 * @argument None
 * @returns String
 */
EH_METHOD(Function, toString) {
	ehretval_p hold_obj;
	if(obj->type() == binding_e) {
		hold_obj = obj;
		obj = obj->get_bindingval()->method;
	}
	ASSERT_OBJ_TYPE(func_e, "Function.toString");
	ehfunc_t *f = obj->get_funcval();
	if(f->type == lib_e) {
		return ehretval_t::make_string(strdup("(args) => (native code)"));
	} else {
		std::ostringstream out;
		out << f->args->decompile(0) << " => (user code)";
		return ehretval_t::make_string(strdup(out.str().c_str()));
	}
}

/*
 * @description Binds the current function to the specified object as the this
 * object. Because the current internal object system of the interpreter is
 * less than ideal, this is a dangerous operation, especially on C++ functions,
 * and its use may lead to crashes.
 * @argument Object to bind to
 * @returns New function
 */
EH_METHOD(Function, bindTo) {
	//obj = ehretval_t::self_or_data(obj);

	if(obj->type() == binding_e) {
		ehbinding_t *b = obj->get_bindingval();
		return ehi->get_parent()->make_binding(new ehbinding_t(args, b->method));
	} else if(obj->is_a(func_e)) {
		return ehi->get_parent()->make_binding(new ehbinding_t(args, obj));
	} else {
		throw_TypeError("Invalid base object for Function.bindTo", obj->type(), ehi);
		return NULL;
	}
}
