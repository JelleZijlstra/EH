/*
 * Binding
 * Binding of a function to an object (this pointer). Provides virtually the
 * same interface as Function.
 */

#include "Binding.hpp"

#include "Function.hpp"
#include "MiscellaneousError.hpp"

ehval_p Binding::make(ehval_p obj, ehval_p method, EHInterpreter *parent) {
	return parent->allocate<Binding>(new Binding::t(obj, method));
}

EH_INITIALIZER(Binding) {
	REGISTER_STATIC_METHOD_RENAME(Binding, new, "operator()");
	REGISTER_METHOD_RENAME(Binding, operator_colon, "operator()");
	REGISTER_METHOD(Binding, toString);
	REGISTER_METHOD(Binding, decompile);
	REGISTER_METHOD(Binding, bindTo);
	REGISTER_METHOD(Binding, method);
	REGISTER_METHOD(Binding, object);
}

/*
 * @description Throws an exception: creating a binding directly is not allowed.
 * @argument N/A
 * @returns N/A
 */
EH_METHOD(Binding, new) {
	throw_MiscellaneousError("Creating a binding directly is not allowed", ehi);
}

/*
 * @description "Decompiles" the method. See also Function.decompile.
 * @argument None
 * @returns String
 */
EH_METHOD(Binding, decompile) {
	ASSERT_OBJ_TYPE(Binding, "Binding.decompile");
	std::string reduction = obj->get<Binding>()->method->decompile(0);
	return String::make(strdup(reduction.c_str()));
}

/*
 * @description Binds a binding to a different object.
 * @argument Object to bind to
 * @returns New binding
 */
EH_METHOD(Binding, bindTo) {
	ASSERT_RESOURCE(Binding, "Binding.bindTo");
	return Binding::make(args, data->method, ehi->get_parent());
}

/*
 * @description Provides a string representation of a binding. Behavior is the
 * same as that of Function.toString.
 * @argument None
 * @returns String
 */
EH_METHOD(Binding, toString) {
	ASSERT_RESOURCE(Binding, "Binding.toString");
	return ehlm_Function_toString(data->method, nullptr, ehi);
}

/*
 * @description Calls a binding.
 * @argument Method arguments
 * @returns Method's return value
 */
EH_METHOD(Binding, operator_colon) {
	ASSERT_RESOURCE(Binding, "Binding.operator()");
	return Function::exec(data->object_data, data->method, args, ehi);
}

/*
 * @description Returns a binding's object.
 * @argument None
 * @returns Object
 */
EH_METHOD(Binding, object) {
	ASSERT_RESOURCE(Binding, "Binding.object");
	return data->object_data;
}

/*
 * @description Returns a binding's function object.
 * @argument None
 * @returns Function
 */
EH_METHOD(Binding, method) {
	ASSERT_RESOURCE(Binding, "Binding.method");
	return data->method;
}
