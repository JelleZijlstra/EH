/*
 * Binding
 * Binding of a function to an object (this pointer). Provides virtually the
 * same interface as Function.
 */

#include "Binding.hpp"
#include "MiscellaneousError.hpp"

ehval_p Binding::make(ehval_p obj, ehval_p method, EHInterpreter *parent) {
	return parent->allocate<Binding>(new Binding::t(obj, method));
}

EH_INITIALIZER(Binding) {
	REGISTER_METHOD_RENAME(Binding, operator_colon, "operator()");
	REGISTER_METHOD(Binding, toString);
	REGISTER_METHOD(Binding, decompile);
	REGISTER_METHOD(Binding, bindTo);
	REGISTER_METHOD(Binding, new);
}

/*
 * @description Throws an exception: creating a binding directly is not allowed.
 * @argument N/A
 * @returns N/A
 */
EH_METHOD(Binding, new) {
	throw_MiscellaneousError("Creating a binding directly is not allowed", ehi);
	return nullptr;
}

/*
 * @description "Decompiles" the method. See also Function.decompile.
 * @argument None
 * @returns String
 */
EH_METHOD(Binding, decompile) {
	ASSERT_OBJ_TYPE(Binding, "Binding.decompile");
	std::string reduction = obj->get<Binding>()->method->data()->decompile(0);
	return String::make(strdup(reduction.c_str()));
}

/*
 * @description Binds a binding to a different object.
 * @argument Object to bind to
 * @returns New binding
 */
EH_METHOD(Binding, bindTo) {
	ASSERT_OBJ_TYPE(Binding, "Binding.bindTo");
	Binding::t *b = obj->get<Binding>();
	return Binding::make(args, b->method, ehi->get_parent());
}

/*
 * @description Provides a string representation of a binding. Behavior is the
 * same as that of Function.toString.
 * @argument None
 * @returns String
 */
EH_METHOD(Binding, toString) {
	ASSERT_OBJ_TYPE(Binding, "Binding.toString");
	return ehlm_Function_toString(obj->get<Binding>()->method, nullptr, ehi);
}

/*
 * @description Calls a binding.
 * @argument Method arguments
 * @returns Method's return value
 */
EH_METHOD(Binding, operator_colon) {
	ASSERT_OBJ_TYPE(Binding, "Binding.operator()");
	Binding::t *binding = obj->get<Binding>();
	return Function::exec(binding->object_data, binding->method, args, ehi);
}
