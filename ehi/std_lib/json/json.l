%{
#include "../std_lib_includes.hpp"
#include "eh_json.bison.hpp"
#include "json.hpp"

#define SCANNER eh_json_get_extra(yyscanner)

EH_NORETURN void eh_json_error(void *, const char *s);
%}
%option yylineno
%option reentrant
%option bison-bridge
%option outfile="eh_json.flex.cpp"
%option stack
%option extra-type="json_scanner *"
%option prefix="eh_json_"

%x JSON_STRING
%%
	BEGIN(INITIAL);

\"			{
				BEGIN(JSON_STRING);
				SCANNER->str_str.str("");
			}
<JSON_STRING>{
	[^\\"]		SCANNER->str_str << yytext;
	\\n			SCANNER->str_str << '\n';
	\\b			SCANNER->str_str << '\b';
	\\f			SCANNER->str_str << '\f';
	\\r			SCANNER->str_str << '\r';
	\\t			SCANNER->str_str << '\t';
	\\u[0-9a-f]{4} {
					eh_json_error(SCANNER, "EH-JSON does not yet support Unicode escape sequences");
				}
	\\[\\"]		SCANNER->str_str << yytext[1];
	\\\/		SCANNER->str_str << '/';
	\\.			{
					char message[100];
					sprintf(message, "Unexpected escape character: %c", yytext[1]);
					eh_json_error(yyscanner, message);
				}
	\"			{
					*yylval_param = String::make(strdup(SCANNER->str_str.str().c_str()));
					BEGIN(INITIAL);
					return T_STRING;
				}
}
-?(0|[1-9][0-9]?) {
				*yylval_param = Integer::make(strtol(yytext, NULL, 10));
				return T_INTEGER;
			}
-?(0|[1-9][0-9]?)(\.[0-9]+)?([eE][+-][0-9]+)? {
				*yylval_param = Float::make(strtod(yytext, NULL));
				return T_FLOAT;
			}
"null"		{
				*yylval_param = Null::make();
				return T_NULL;
			}
"true"		{
				*yylval_param = Bool::make(true);
				return T_BOOL;
			}
"false"		{
				*yylval_param = Bool::make(false);
				return T_BOOL;
			}
[\[\{\"\:\}\,\]]	return *yytext;
[ \t\n\r]+		/* Ignore whitespace */
.			{
				char message[100];
				sprintf(message, "Unexpected character: %c", *yytext);
				eh_json_error(yyscanner, message);
			}
%%
extern "C" int eh_json_wrap(yyscan_t) {
	return 1;
}
