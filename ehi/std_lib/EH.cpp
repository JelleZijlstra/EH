/*
 * EH
 * Module for methods that allow for interactions with the EH engine.
 */

#include "EH.hpp"

EH_INITIALIZER(EH) {
	REGISTER_METHOD(EH, eval);
	REGISTER_METHOD(EH, collectGarbage);
	REGISTER_METHOD(EH, contextName);
	REGISTER_METHOD(EH, parse);
}

/*
 * @description Parse and execute arbitrary EH code at runtime.
 * @argument String containing code to execute
 * @returns Nothing useful
 */
EH_METHOD(EH, eval) {
	ehval_p arg = ehi->toString(args, obj);
	ehobj_t *scope_obj = new ehobj_t();
	ehval_p scope = Object::make(scope_obj, ehi->get_parent());
	scope_obj->type_id = 1; // Object
	scope_obj->parent = ehi->global();
	ehi->spawning_parse_string(arg->get<String>(), scope);
	return scope;
}

/*
 * @description Run the garbage collector. This is currently a dangerous
 * operation that is likely to cause the engine to crash.
 * @argument None
 * @returns Null
 */
EH_METHOD(EH, collectGarbage) {
	ehi->get_parent()->gc.do_collect();
	return Null::make();
}

/*
 * @description Return the name of the current context (e.g., a file name).
 * @argument None
 * @returns String
 */
EH_METHOD(EH, contextName) {
	ASSERT_NULL("contextName");
	return String::make(strdup(ehi->get_context_name().c_str()));
}

/*
 * @description Parses a string into a piece of AST.
 * @argument String to parse
 * @returns Node
 */
EH_METHOD(EH, parse) {
	ASSERT_TYPE(args, String, "EH.parse");
	const char *cmd = args->get<String>();
	EHI parser(end_is_end_e, ehi->get_parent(), ehi->get_context(), ehi->get_working_dir(), "(eval'd code)");
	parser.parse_string(cmd);
	return parser.get_code();
}
