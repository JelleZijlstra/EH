#include <sstream>

#include "Function.hpp"

EH_INITIALIZER(Function) {
	REGISTER_METHOD_RENAME(Function, operator_colon, "operator()");
	REGISTER_METHOD(Function, toString);
	REGISTER_METHOD(Function, decompile);
	REGISTER_METHOD(Function, bindTo);
}

EH_METHOD(Function, operator_colon) {
	// This is probably the most important library method in EH. It works
	// on both Function and binding objects.
	ehretval_p base_object;
	ehretval_p function_object;
	ehretval_p parent;
	if(obj->is_a(func_e)) {
		function_object = obj;
		parent = obj->get_objectval()->parent;
		base_object = ehi->global();
	} else if(obj->type() == binding_e) {
		ehbinding_t *binding = obj->get_bindingval();
		function_object = binding->method;
		parent = binding->method->get_objectval()->parent;
		base_object = binding->object_data;
	} else {
		throw_TypeError("Invalid base object for Function.operator()", obj->type(), ehi);
	}
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

EH_METHOD(Function, bindTo) {
	if(obj->type() == binding_e) {
		ehbinding_t *b = obj->get_bindingval();
		return ehi->get_parent()->make_binding(new ehbinding_t(args, b->method));
	} else if(obj->type() == func_e) {
		return ehi->get_parent()->make_binding(new ehbinding_t(args, obj));
	} else {
		throw_TypeError("Invalid base object for Function.bindTo", obj->type(), ehi);
		return NULL;
	}
}
