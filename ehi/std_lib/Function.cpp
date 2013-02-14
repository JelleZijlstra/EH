/*
 * Function
 * Represents functions. Internally, functions are in fact often represented as
 * bindings, pairs of an object (which will be the this object inside the
 * function) and the function itself. There is no Function constructor.
 */

#include <sstream>

#include "Function.hpp"
#include "ArgumentError.hpp"
#include "Binding.hpp"
#include "MiscellaneousError.hpp"
#include "SuperClass.hpp"

EH_INITIALIZER(Function) {
	REGISTER_METHOD_RENAME(Function, operator_colon, "operator()");
	REGISTER_METHOD(Function, toString);
	REGISTER_METHOD(Function, decompile);
	REGISTER_METHOD(Function, bindTo);
	REGISTER_METHOD(Function, args);
	REGISTER_METHOD(Function, code);
}

/*
 * @description Call a function. Overriding this method is not possible; it is
 * called directly by the engine, rather than using the normal call mechanisms.
 * @argument Arguments to the function call
 * @returns Return value of the function
 */
EH_METHOD(Function, operator_colon) {
	ASSERT_OBJ_TYPE(Function, "Function.operator()");
	return Function::exec(ehi->global(), _obj, args, ehi);
}

ehval_p Function::exec(ehval_p base_object, ehval_p function_object, ehval_p args, EHI *ehi) {
	Function::t *f = function_object->data()->get<Function>();

	if(base_object->is_a<SuperClass>()) {
		base_object = base_object->get<SuperClass>();
	}

	switch(f->type) {
		case lib_e:
			return f->libmethod_pointer(base_object, args, ehi);
		case compiled_e: {
			ehval_p newcontext = ehi->get_parent()->instantiate(function_object);
			newcontext->get<Object>()->object_data = function_object->get<Object>()->object_data;
			return f->compiled_pointer(base_object, args, ehi, ehcontext_t(base_object, newcontext));
		}
		case user_e: {
			ehval_p newcontext = ehi->get_parent()->instantiate(function_object);
			newcontext->get<Object>()->object_data = function_object->get<Object>()->object_data;

			// set arguments
			attributes_t attributes(private_e, nonstatic_e, nonconst_e);
			ehi->set(f->args, args, &attributes, ehcontext_t(base_object, newcontext));

			// execute the function
			ehval_p ret = ehi->eh_execute(f->code, ehcontext_t(base_object, newcontext));
			ehi->not_returning();
			return ret;
		}
	}
}

ehval_p Function::make(t *val, EHInterpreter *parent) {
	return parent->allocate<Function>(val);
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
	ASSERT_OBJ_TYPE(Function, "Function.toString");
	Function::t *f = obj->get<Function>();
	if(f->type == Function::lib_e) {
		return String::make(strdup("(args) => (native code)"));
	} else {
		std::ostringstream out;
		out << f->args->decompile(0) << " => (user code)";
		return String::make(strdup(out.str().c_str()));
	}
}

/*
 * @description Binds the current function to the specified object as the this
 * object.
 * @argument Object to bind to
 * @returns New function
 */
EH_METHOD(Function, bindTo) {
	ASSERT_OBJ_TYPE(Function, "Function.bindTo");
	return Binding::make(args, _obj, ehi->get_parent());
}

/*
 * @description Returns the arguments for this function as an AST fragment.
 * @argument None
 * @returns Node
 */
EH_METHOD(Function, args) {
	ASSERT_RESOURCE(Function, "Function.args");
	if(data->type == Function::user_e) {
		return data->args;
	} else {
		throw_ArgumentError("Cannot give arguments for non-user function", "Function.args", obj, ehi);
	}
}

/*
 * @description Returns the code for this function as an AST fragment.
 * @argument None
 * @returns Node
 */
EH_METHOD(Function, code) {
	ASSERT_RESOURCE(Function, "Function.code");
	if(data->type == Function::user_e) {
		return data->code;
	} else {
		throw_ArgumentError("Cannot give code for non-user function", "Function.code", obj, ehi);
	}
}
