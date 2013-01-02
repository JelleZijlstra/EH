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
	ehval_p base_object;
	ehval_p function_object;
	if(obj->deep_is_a<Function>()) {
		function_object = obj;
		base_object = ehi->global();
	} else if(obj->is_a<Binding>()) {
		Binding::t *binding = obj->get<Binding>();
		function_object = binding->method;
		base_object = binding->object_data;
	} else {
		throw_TypeError("Invalid base object for Function.operator()", obj, ehi);
	}
	return Function::exec(base_object, function_object, args, ehi);
}

ehval_p Function::exec(ehval_p base_object, ehval_p function_object, ehval_p args, EHI *ehi) {
	Function::t *f = function_object->data()->get<Function>();

	if(f->type == lib_e) {
		return f->libmethod_pointer(base_object, args, ehi);
	}
	ehval_p newcontext = ehi->get_parent()->instantiate(function_object);
	newcontext->get<Object>()->object_data = function_object->get<Object>()->object_data;

	// set arguments
	attributes_t attributes = attributes_t::make(private_e, nonstatic_e, nonconst_e);
	ehi->set(f->args, args, &attributes, ehcontext_t(base_object, newcontext));

	// execute the function
	ehval_p ret = ehi->eh_execute(f->code, ehcontext_t(base_object, newcontext));
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
	ehval_p hold_obj;
	if(obj->is_a<Binding>()) {
		hold_obj = obj;
		obj = obj->get<Binding>()->method;
	}
	ASSERT_OBJ_TYPE(Function, "Function.decompile");
	std::string reduction = obj->decompile(0);
	return String::make(strdup(reduction.c_str()));
}

/*
 * @description Provides a string representation of a function. For C++
 * functions, this is simply "(args) => (native code)"; for user functions,
 * the argument list is decompiled.
 * @argument None
 * @returns String
 */
EH_METHOD(Function, toString) {
	ehval_p hold_obj;
	if(obj->is_a<Binding>()) {
		hold_obj = obj;
		obj = obj->get<Binding>()->method;
	}
	ASSERT_OBJ_TYPE(Function, "Function.toString");
	Function::t *f = obj->get<Function>();
	if(f->type == lib_e) {
		return String::make(strdup("(args) => (native code)"));
	} else {
		std::ostringstream out;
		out << f->args->decompile(0) << " => (user code)";
		return String::make(strdup(out.str().c_str()));
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
	if(obj->is_a<Binding>()) {
		Binding::t *b = obj->get<Binding>();
		return Binding::make(args, b->method, ehi->get_parent());
	} else if(obj->deep_is_a<Function>()) {
		return Binding::make(args, obj, ehi->get_parent());
	} else {
		throw_TypeError("Invalid base object for Function.bindTo", obj, ehi);
		return nullptr;
	}
}

ehval_p Binding::make(ehval_p obj, ehval_p method, EHInterpreter *parent) {
	return parent->allocate<Binding>(new Binding::t(obj, method));
}

EH_INITIALIZER(Binding) {
	REGISTER_METHOD_RENAME(Binding, operator_colon, "operator()");
	REGISTER_METHOD(Binding, toString);
	REGISTER_METHOD(Binding, decompile);
	REGISTER_METHOD(Binding, bindTo);
}

#define BINDING_METHOD(name) EH_METHOD(Binding, name) { \
	ASSERT_RESOURCE(Binding, "Binding." #name); \
	return ehlm_Function_ ## name(obj, args, ehi); \
}

BINDING_METHOD(operator_colon)
BINDING_METHOD(toString)
BINDING_METHOD(decompile)
BINDING_METHOD(bindTo)
