%{
#include <stdlib.h>
#include <stdio.h>
#include "eh.h"
#include "y.tab.h"
%}
%option yylineno
%%
['][^']*[']	{
				int len;
				len = strlen(yytext);
				yylval.sValue = Malloc(len - 1);
				strncpy(yylval.sValue, yytext + 1, len - 2);
				yylval.sValue[len - 2] = '\0';
				return T_STRING;
			}
[0-9]*	{ yylval.iValue = atoi(yytext); return T_INTEGER; }
"int"		{
				yylval.tValue = int_e;
				return T_TYPE;
			}
"string"	{
				yylval.tValue = string_e;
				return T_TYPE;
			}
"array"		{
				yylval.tValue = array_e;
				return T_TYPE;
			}
"bool"		{
				yylval.tValue = bool_e;
				return T_TYPE;
			}
"echo"		return T_ECHO;
"if"		return T_IF;
"else"		return T_ELSE;
"endif"		return T_ENDIF;
"for"		return T_FOR;
"endfor"	return T_ENDFOR;
"count"		return T_COUNT;
"while"		return T_WHILE;
"endwhile"	return T_ENDWHILE;
"break"		return T_BREAK;
"call"		return T_CALL;
"func"		return T_FUNC;
"endfunc"	return T_ENDFUNC;
"ret"		return T_RET;
"null"		return T_NULL;
"class"		return T_CLASS;
"endclass"	return T_ENDCLASS;
"new"		return T_NEW;
"global"	return T_GLOBAL;
"&&"		return T_AND;
"and"		return T_AND;
"||"		return T_OR;
"or"		return T_OR;
"xor"		return T_XOR;
"public"	{
				yylval.vValue = public_e;
				return T_VISIBILITY;
			}
"private"	{
				yylval.vValue = private_e;
				return T_VISIBILITY;
			}
"true"		{
				yylval.bValue = true;
				return T_BOOL;
			}
"false"		{
				yylval.bValue = false;
				return T_BOOL;
			}
"$ "		return T_SET;
"++"		return T_PLUSPLUS;
"--"		return T_MINMIN;
("//"|#).*[;\n]	; /* Ignore comment */
[;\n]+		return T_SEPARATOR;
[a-zA-Z][a-zA-Z0-9]*	{
				int len;
				len = strlen(yytext);
				yylval.sValue = Malloc(len + 1);
				strncpy(yylval.sValue, yytext, len);
				yylval.sValue[len] = '\0';
				return T_VARIABLE;
			}
">="		return T_GE;
"<="		return T_LE;
"!="		return T_NE;
"=="		return T_SE;
"=>"		return T_DOUBLEARROW;
"->"		{
				yylval.aValue = arrow_e;
				return T_ACCESSOR;
			}
"\."		{
				yylval.aValue = dot_e;
				return T_ACCESSOR;
			}
[@=$+\-*/<>(),:\[\]&\|^%~!]	return *yytext; /* Single character tokens */
[ \t]+		; /* Ignore whitespace */
.			{
				char message[100];
				sprintf(message, "Unexpected character %s", yytext);
				yyerror(message);
			}
%%
int yywrap(void) {
	return 1;
}