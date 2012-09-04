#include <sstream>

#include "Function.h"

START_EHLC(Function)
EHLC_ENTRY_RENAME(Function, operator_colon, "operator:")
EHLC_ENTRY(Function, toString)
END_EHLC()

EH_METHOD(Function, operator_colon) {
	// This is probably the most important library method in EH. It works
	// on both Function and binding objects.
	ehretval_p base_object;
	ehretval_p function_object;
	ehretval_p parent;
	ehretval_p real_parent;
	if(obj->is_a(func_e)) {
		function_object = obj;
		parent = obj->get_objectval()->parent;
		real_parent = obj->get_objectval()->real_parent;
		base_object = ehi->global_object;
	} else if(obj->type() == binding_e) {
		ehbinding_t *binding = obj->get_bindingval();
		function_object = binding->method;
		parent = binding->method->get_objectval()->parent;
		real_parent = binding->method->get_objectval()->real_parent;
		base_object = binding->object_data;
	} else {
		throw_TypeError("Invalid base object for Function.operator:", obj->type(), ehi);
	}
	ehfunc_t *f = function_object->get_objectval()->object_data->get_funcval();

	if(f->type == lib_e) {
		return f->libmethod_pointer(base_object, args, ehi);
	}
	ehretval_p newcontext = ehi->object_instantiate(function_object);
	newcontext->get_objectval()->object_data = function_object->get_objectval()->object_data;

	// check parameter count
	if(f->argcount == 0) {
		if(args->type() != null_e) {
			throw_TypeError("Unexpected non-null argument to closure", args->type(), ehi);
		}
	} else if(f->argcount == 1) {
		ehi->set_property(newcontext, f->args[0].name.c_str(), args, newcontext);
	} else {
		if(args->type() != tuple_e) {
			std::ostringstream msg;
			msg << "Argument must be a tuple of size " << f->argcount;
			throw_ArgumentError(msg.str().c_str(), "(closure)", args, ehi);
		} else if(args->get_tupleval()->size() != f->argcount) {
			std::ostringstream msg;
			msg << "Argument must be a tuple of size " << f->argcount;
			throw_ArgumentError(msg.str().c_str(), "(closure)", args, ehi);
		} else {
			// set parameters as necessary
			ehtuple_t *tuple = args->get_tupleval();
			for(int i = 0; i < f->argcount; i++) {
				ehi->set_property(newcontext, f->args[f->argcount - 1 - i].name.c_str(), tuple->get(i), newcontext);
			}			
		}
	}	
	ehretval_p ret = ehi->eh_execute(f->code, ehcontext_t(base_object, newcontext));
	ehi->returning = false;
	return ret;
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
		return ehretval_t::make_string(strdup("func: -> (native code)"));
	} else if(f->argcount == 0) {
		return ehretval_t::make_string(strdup("func: -> (user code)"));
	} else {
		std::ostringstream out;
		out << "func: ";
		for(int i = 0; i < f->argcount; i++) {
			out << f->args[i].name;
			if(i != f->argcount - 1) {
				out << ", ";
			}
		}
		out << " -> (user code)";
		return ehretval_t::make_string(strdup(out.str().c_str()));
	}
}
