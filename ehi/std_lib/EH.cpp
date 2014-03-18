/*
 * EH
 * Module for methods that allow for interactions with the EH engine.
 */

#include "EH.hpp"
#include "ArgumentError.hpp"
#include "Array.hpp"
#include "Attribute.hpp"
#include "ByteArray.hpp"
#include "Function.hpp"
#include "Node.hpp"
#include "../eh_vm.hpp"
#include "../eh.bison.hpp"
#include "../eh.flex.hpp"

EH_INITIALIZER(EH) {
	REGISTER_STATIC_METHOD(EH, eval);
	REGISTER_STATIC_METHOD(EH, collectGarbage);
	REGISTER_STATIC_METHOD(EH, contextName);
	REGISTER_STATIC_METHOD(EH, parse);
	REGISTER_STATIC_METHOD(EH, lex);
	REGISTER_STATIC_METHOD(EH, printStack);
	REGISTER_STATIC_METHOD(EH, executeBytecode);
}

/*
 * @description Parse and execute arbitrary EH code at runtime.
 * @argument String containing code to execute
 * @returns Nothing useful
 */
EH_METHOD(EH, eval) {
	ehval_p arg = ehi->toString(args, obj);
	ehval_p scope = Function_Scope::make(ehi->global(), ehi->get_parent());
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

// defined in eh.y
const char *get_token_name(int token);

/*
 * @description Lex a string into a sequence of EH lexer tokens.
 * @argument String to lex
 * @returns Array
 */
EH_METHOD(EH, lex) {
	ASSERT_TYPE(args, String, "EH.lex");
	const char *cmd = args->get<String>();

	// initialize scanner
	void *scanner = NULL;
	EHI new_ehi(end_is_end_e, ehi->get_parent(), ehi->get_context(), ehi->get_working_dir(), "EH.lex");
	yylex_init_extra(&new_ehi, &scanner);
	yy_switch_to_buffer(yy_scan_string(cmd, scanner), scanner);
	yyset_lineno(1, scanner);
	YYSTYPE yylval;

	// initialize output array
	ehval_p out_val = Array::make(ehi->get_parent());
	auto arr = out_val->get<Array>();
	size_t index = 0;

	// perform the lexing
	const char *token_name;
	while(true) {
		int token = yylex(&yylval, scanner);
		// symbolic tokens
		if(token >= 256) {
			token_name = get_token_name(token);
		} else if(token == 0) {
			// EOF
			break;
		} else {
			char single_char[2];
			single_char[0] = token;
			single_char[1] = '\0';
			token_name = single_char;
		}
		arr->append(String::make(strdup(token_name)));
		index++;
	}

	return out_val;
}

/*
 * @description Print the current EH stack
 * @argument None
 * @returns Null
 */
EH_METHOD(EH, printStack) {
	ASSERT_TYPE(args, Null, "EH.printStack");
	for(auto &entry : ehi->get_stack()) {
		std::cout << "- " << entry->name << std::endl;
	}
	return nullptr;
}

/*
 * @description Executes a blob of bytecode
 * @argument ByteArray containing valid EH code object
 * @returns Null
 */
EH_METHOD(EH, executeBytecode) {
	ASSERT_TYPE(args, ByteArray, "EH.executeBytecode");
	auto ba = args->get<ByteArray>();
	size_t ba_size = ba->size;
	size_t header_size = static_cast<code_object_header *>(static_cast<void *>(ba->content))->size;
	if(ba_size != header_size) {
		throw_ArgumentError("bytecode has incorrect size", "EH.executeBytecode", args, ehi);
	}

	eh_execute_bytecode(ba->content, ehi);
	return nullptr;
}
