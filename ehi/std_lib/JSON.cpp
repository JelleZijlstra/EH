/*
 * JSON
 * Provides JSON parsing.
 */
#include "std_lib_includes.hpp"
#include "JSON.hpp"
#include "json/json.hpp"
#include "json/eh_json.bison.hpp"
#include "json/eh_json.flex.hpp"
#include "SyntaxError.hpp"

int eh_json_parse(void *);

EH_INITIALIZER(JSON) {
	REGISTER_METHOD(JSON, parse);
}

EH_METHOD(JSON, parse) {
	ASSERT_TYPE(args, String, "JSON.parse");
	json_scanner js{ehi};
	return js.parse(args->get<String>());
}

ehval_p json_scanner::parse(const char *str) {
	eh_json__switch_to_buffer(eh_json__scan_string(str, inner_scanner), inner_scanner);
	eh_json_set_lineno(1, inner_scanner);
	eh_json_parse(inner_scanner);
	return result;
}

EH_NORETURN void eh_json_error(void *scanner, const char *message) {
	json_scanner *sc = eh_json_get_extra(scanner);
	throw_SyntaxError(message, 0, sc->ehi);
}

json_scanner::json_scanner(EHI *_ehi) : str_str(), ehi(_ehi), result() {
	eh_json_lex_init_extra(this, &inner_scanner);
}

json_scanner::~json_scanner() {
	eh_json_lex_destroy(inner_scanner);
}
