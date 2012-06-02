%{
/*
 * eh.l
 * Jelle Zijlstra, December 2011
 *
 * Lex input specification for the EH grammar.
 */
#include <stdlib.h>
#include <stdio.h>
#include "eh.h"
#include "eh.bison.hpp"
%}
%option yylineno
%option reentrant
%option bison-bridge
%option outfile="eh.flex.cpp"
%%
['][^']*[']	{
				int len;
				len = strlen(yytext);
				yylval_param->sValue = new char[len - 1];
				strncpy(yylval_param->sValue, yytext + 1, len - 2);
				yylval_param->sValue[len - 2] = '\0';
				return T_STRING;
			}
["][^"]*["]	{
				int len;
				len = strlen(yytext);
				yylval_param->sValue = new char[len - 1];
				strncpy(yylval_param->sValue, yytext + 1, len - 2);
				yylval_param->sValue[len - 2] = '\0';
				return T_STRING;
			}
[\\]\n		/* Ignore escape */
[0-9]*\.[0-9]+	{
				yylval_param->fValue = strtof(yytext, NULL);
				return T_FLOAT;
			}
[0-9]+		{
				yylval_param->iValue = atoi(yytext);
					return T_INTEGER;
			}
"int"		{
				yylval_param->tValue = int_e;
				return T_TYPE;
			}
"string"	{
				yylval_param->tValue = string_e;
				return T_TYPE;
			}
"array"		{
				yylval_param->tValue = array_e;
				return T_TYPE;
			}
"bool"		{
				yylval_param->tValue = bool_e;
				return T_TYPE;
			}
"float"		{
				yylval_param->tValue = float_e;
				return T_TYPE;
			}
"range"		{
				yylval_param->tValue = range_e;
				return T_TYPE;
			}
"if"		return T_IF;
"else"		return T_ELSE;
"set"		return T_SET;
"endif"		return T_ENDIF;
"for"		return T_FOR;
"endfor"	return T_ENDFOR;
"as"		return T_AS;
"count"		return T_COUNT;
"while"		return T_WHILE;
"endwhile"	return T_ENDWHILE;
"break"		return T_BREAK;
"continue"	return T_CONTINUE;
"func"		return T_FUNC;
"endfunc"	return T_ENDFUNC;
"ret"		return T_RET;
"null"		return T_NULL;
"class"		return T_CLASS;
"endclass"	return T_ENDCLASS;
"switch"	return T_SWITCH;
"endswitch"	return T_ENDSWITCH;
"given"		return T_GIVEN;
"end"		return T_END;
"case"		return T_CASE;
"default"	return T_DEFAULT;
"new"		return T_NEW;
"::"		{
				yylval_param->aValue = doublecolon_e;
				return T_ACCESSOR;
			}
"&&"		return T_AND;
"and"		return T_AND;
"||"		return T_OR;
"or"		return T_OR;
"^^"		return T_XOR;
"xor"		return T_XOR;
"public"	{
				yylval_param->vValue = publica_e;
				return T_ATTRIBUTE;
			}
"private"	{
				yylval_param->vValue = privatea_e;
				return T_ATTRIBUTE;
			}
"static"	{
				yylval_param->vValue = statica_e;
				return T_ATTRIBUTE;
			}
"const"		{
				yylval_param->vValue = consta_e;
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
"$("		return T_DOLLARPAREN;
"$ "		return T_SET;
"++"		return T_PLUSPLUS;
"--"		return T_MINMIN;
":="		return T_ASSIGNMENT;
("//"|#).*[;\n]	; /* Ignore comment */
[;\n]		return T_SEPARATOR;
[a-zA-Z][a-zA-Z0-9_]*	{
				int len;
				len = strlen(yytext);
				yylval_param->sValue = new char[len + 1];
				strncpy(yylval_param->sValue, yytext, len);
				yylval_param->sValue[len] = '\0';
				return T_VARIABLE;
			}
".."		return T_RANGE;
">="		return T_GE;
"<="		return T_LE;
"!="		return T_NE;
"!=="		return T_SNE;
"=="		return T_SE;
"=>"		return T_DOUBLEARROW;
"->"		{
				yylval_param->aValue = arrow_e;
				return T_ACCESSOR;
			}
[@=$+\-*/<>(),:\[\]&\|^%~!{}\.]	return *yytext; /* Single character tokens */
[ \t]+		; /* Ignore whitespace */
.			{
				char message[100];
				sprintf(message, "Unexpected character %s", yytext);
				yyerror(yyscanner, message);
			}
%%
int yywrap(void *scanner) {
	if(is_interactive) {
		char *buf;
		buf = interpreter->eh_getline();
		if(!buf)
			return 1;
		yy_scan_string(buf, scanner);
		return 0;
	}
	return 1;
}

int yyparse(void *);
ehretval_t EHParser::parse_file(FILE *infile) {
	ehretval_t ret;

	buffer = yy_create_buffer(infile, YY_BUF_SIZE, scanner);
	yy_switch_to_buffer(buffer, scanner);
	ret.type = int_e;
	ret.intval = yyparse(scanner);
	return ret;
}
ehretval_t EHParser::parse_string(char *cmd) {
	ehretval_t ret;

	yy_scan_string(cmd, scanner);

	ret.type = int_e;
	ret.intval = yyparse(scanner);
	return ret;
}
EHParser::EHParser(void) {
	yylex_init(&scanner);
}
EHParser::~EHParser(void) {
	yylex_destroy(scanner);
}
void yyerror(void *scanner, const char *s) {
	// give up on line numbers for now
	eh_error(s, eparsing_e);
}
