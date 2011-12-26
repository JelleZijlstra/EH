%{
#include <stdlib.h>
#include <stdio.h>
#include "eh.h"
#include "y.tab.h"
%}
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
"echo"		return T_ECHO;
"if"		return T_IF;
"else"		return T_ELSE;
"endif"		return T_ENDIF;
"for"		return T_FOR;
"endfor"	return T_ENDFOR;
"count"		return T_COUNT;
"while"		return T_WHILE;
"endwhile"	return T_ENDWHILE;
"call"		return T_CALL;
"func"		return T_FUNC;
"endfunc"	return T_ENDFUNC;
"ret"		return T_RET;
"null"		return T_NULL;
"$ "		return T_SET;
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
"->"		return T_ARROW;
[@=$+\-*/<>(),:\[\]]	return *yytext; /* Single character tokens */
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