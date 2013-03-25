%{
/*
 * eh.l
 * Jelle Zijlstra, December 2011
 *
 * Lex input specification for the EH grammar.
 */
#include "eh.hpp"
#include "std_lib/SyntaxError.hpp"
#include "std_lib/Integer.hpp"
#include "std_lib/Attribute.hpp"
#include "std_lib/Node.hpp"
#include "eh.bison.hpp"

void make_string(char **, char *);
void yyerror(void *, const char *s);
void copy_string(char **, char *);

#define COPY_STRING() do { \
	size_t len = strlen(yytext); \
	yylval_param->sValue = new char[len + 1]; \
	strncpy(yylval_param->sValue, yytext, len); \
	yylval_param->sValue[len] = '\0'; \
} while(0)

%}
%option yylineno
%option reentrant
%option bison-bridge
%option outfile="eh.flex.cpp"
%option extra-type="class EHI *"

OPCHAR		[@=$+\-*/<>,:&\|^%~!\.]
%%
['][^']*[']	{
				make_string(&yylval_param->sValue, yytext);
				return T_STRING;
			}
["][^"]*["]	{
				make_string(&yylval_param->sValue, yytext);
				return T_STRING;
			}
[\\]\n		/* Ignore escape */
-?[0-9]*\.[0-9]+	{
				yylval_param->fValue = strtof(yytext, nullptr);
				return T_FLOAT;
			}
-?[0-9]+		{
				yylval_param->iValue = atoi(yytext);
				return T_INTEGER;
			}
"if"		return T_IF;
"else"		return T_ELSE;
"elsif"		return T_ELSIF;
"for"		return T_FOR;
"as"		return T_AS;
"in"		return T_IN;
"while"		return T_WHILE;
"break"		return T_BREAK;
"continue"	return T_CONTINUE;
"func"		return T_FUNC;
"ret"		return T_RET;
"null"		return T_NULL;
"()"		return T_NULL;
"class"		return T_CLASS;
"switch"	return T_SWITCH;
"given"		return T_GIVEN;
"match"		return T_MATCH;
"end"		return T_END;
"case"		return T_CASE;
"default"	return T_DEFAULT;
"try"		return T_TRY;
"catch"		return T_CATCH;
"finally"	return T_FINALLY;
"this"		return T_THIS;
"scope"		return T_SCOPE;
"enum"		return T_ENUM;
"raw"		return T_RAW;
"when"		return T_WHEN;
"public"	{
				yylval_param->vValue = Attribute::publica_e;
				return T_ATTRIBUTE;
			}
"private"	{
				yylval_param->vValue = Attribute::privatea_e;
				return T_ATTRIBUTE;
			}
"static"	{
				yylval_param->vValue = Attribute::statica_e;
				return T_ATTRIBUTE;
			}
"const"		{
				yylval_param->vValue = Attribute::consta_e;
				return T_ATTRIBUTE;
			}
"true"		{
				yylval_param->bValue = true;
				return T_BOOL;
			}
"false"		{
				yylval_param->bValue = false;
				return T_BOOL;
			}
"++"		return T_PLUSPLUS;
"--"		return T_MINMIN;
("//"|#).*	; /* Ignore comment */
[;\n]		return T_SEPARATOR;
"&&"		return T_AND;
"and"		return T_AND;
"||"		return T_OR;
"or"		return T_OR;
"^^"		return T_XOR;
"xor"		return T_XOR;
".."		return T_RANGE;
"=>"		return T_DOUBLEARROW;
"->"		return T_ARROW;
("<<"|">>")	{ COPY_STRING(); return T_SHIFT_OP; }
"operator"({OPCHAR}+|\(\))	{
				size_t len = strlen(yytext);
				yylval_param->sValue = new char[len + 1];
				strncpy(yylval_param->sValue, yytext, len);
				yylval_param->sValue[len] = '\0';
				return T_VARIABLE;
			}
[a-zA-Z][a-zA-Z0-9_]*	{
				size_t len = strlen(yytext);
				yylval_param->sValue = new char[len + 1];
				strncpy(yylval_param->sValue, yytext, len);
				yylval_param->sValue[len] = '\0';
				return T_VARIABLE;
			}
[@$(),\-\|:\[\]~!{}\._=]	{
				return *yytext; /* Single character tokens */
			}
(">="|"<=") { COPY_STRING(); return T_COMPARE_OP; }
("!="|"==")	{ COPY_STRING(); return T_EQ_OP; }
{OPCHAR}+"="	{
				// Get rid of last character
				size_t len = strlen(yytext) - 1;
				yylval_param->sValue = new char[len + 1];
				strncpy(yylval_param->sValue, yytext, len);
				yylval_param->sValue[len] = '\0';
				return T_COMPOUND_ASSIGN;
			}
"|"{OPCHAR}+	{ COPY_STRING(); return T_BAR_OP; }
"^"{OPCHAR}*	{ COPY_STRING(); return T_CARET_OP; }
"&"{OPCHAR}*	{ COPY_STRING(); return T_AMPERSAND_OP; }
[<>]{OPCHAR}*	{ COPY_STRING(); return T_COMPARE_OP; }
[!=]{OPCHAR}*	{ COPY_STRING(); return T_EQ_OP; }
":"{OPCHAR}+	{ COPY_STRING(); return T_COLON_OP; }
[+-]{OPCHAR}*	{ COPY_STRING(); return T_PLUS_OP; }
[*/%]{OPCHAR}*	{ COPY_STRING(); return T_MULT_OP; }
{OPCHAR}+	{ COPY_STRING(); return T_CUSTOM_OP; }
[ \t]+		; /* Ignore whitespace */
.			{
				char message[100];
				sprintf(message, "Unexpected character %s", yytext);
				yyerror(yyscanner, message);
			}
%%
int yywrap(yyscan_t scanner) {
	EHI *parser = yyget_extra(scanner);
	if(parser->get_interactivity() != end_is_end_e) {
		char *buf = parser->eh_getline();
		if(!buf) {
			return 1;
		}
		yy_scan_string(buf, scanner);
		return 0;
	}
	return 1;
}

void yyerror(void *scanner, const char *s) {
	EHI *ehi = yyget_extra(scanner);
	throw_SyntaxError(s, yyget_lineno((yyscan_t)scanner), ehi);
}

void make_string(char **dest, char *src) {
	src++;
	size_t len = strlen(src) - 1;
	char *deststr = new char[len + 1];
	int dest_index = 0;
	for(unsigned int i = 0; i < len; i++) {
		if(src[i] == '\\') {
			if(i == len - 1) {
				deststr[dest_index] = '\\';
			} else {
				i++;
				char c = src[i];
				switch(c) {
					case 'n':
						deststr[dest_index] = '\n';
						break;
					case 'r':
						deststr[dest_index] = '\b';
						break;
					case '0':
						deststr[dest_index] = '\0';
						break;
					case 'b':
						deststr[dest_index] = '\b';
						break;
					default:
						deststr[dest_index] = c;
						break;
				}
			}
		} else {
			deststr[dest_index] = src[i];
		}
		dest_index++;
	}
	deststr[dest_index] = '\0';
	*dest = deststr;
}
