%{
#include <stdlib.h>
#include <stdio.h>
#include "y.tab.h"
#include "eh.h"
%}
%%
['].*[']	{ 
				int len;
				len = strlen(yytext);
				strncpy(yylval.sValue, yytext + 1, len - 2);
				yylval.sValue[len - 2] = '\0';
				return T_STRING; 
			}
[1-9][0-9]*	{ yylval.iValue = atoi(yytext); return T_INTEGER; }
"echo"		return T_ECHO;
"if"		return T_IF;
"endif"		return T_ENDIF;
"//".*[;\n]	; /* Ignore comment */
[;\n]+		return T_SEPARATOR;
[=$+\-*/<>()]	return *yytext;
">="		return T_GE;
"<="		return T_LE;
"!="		return T_NE;
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