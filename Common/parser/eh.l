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
				yylval.sValue = Malloc(len - 1);
				strncpy(yylval.sValue, yytext + 1, len - 2);
				yylval.sValue[len - 2] = '\0';
				return T_STRING; 
			}
[0-9]*	{ yylval.iValue = atoi(yytext); return T_INTEGER; }
"echo"		return T_ECHO;
"if"		return T_IF;
"else"		return T_ELSE;
"endif"		return T_ENDIF;
"while"		return T_WHILE;
"endwhile"	return T_ENDWHILE;
"call"		return T_CALL;
"func"		return T_FUNC;
"endfunc"	return T_ENDFUNC;
"ret"		return T_RET;
"$ "		return T_SET;
"//".*[;\n]	; /* Ignore comment */
[;\n]+		return T_SEPARATOR;
[a-zA-Z][a-zA-Z0-9]*	{ 
				int len;
				len = strlen(yytext);
				yylval.sValue = Malloc(len + 1);
				strncpy(yylval.sValue, yytext, len);
				yylval.sValue[len] = '\0';
				return T_VARIABLE; 
			}
[=$+\-*/<>(),:]	return *yytext; /* Single character tokens */
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