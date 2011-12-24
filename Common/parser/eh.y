/* This code was inspired by Tom Niemann's "A Compact Guide to Lex & Yacc", available at http://epaperpress.com/lexandyacc/ */
%{
#include "eh.h"

%}
%union {
	char sValue[100];
	int iValue;
	struct ehnode_t *ehNode;
};
%token <iValue> T_INTEGER
%token T_IF
%token T_ENDIF
%token T_WHILE
%token T_ENDWHILE
%token T_ECHO
%token T_SEPARATOR
%token <sValue> T_STRING
%left '=' '>' '<' T_GE T_LE T_NE
%left '+' '-'
%left '*' '/'
%nonassoc '(' ')'

%type<ehNode> statement expression statement_list string
%%
program:
	statement_list			{ execute($1); free_node($1); exit(0); }
	| /* NULL */			{ 
								fprintf(stderr, "No input");
								exit(1);
							}
	;

statement_list:
	statement				{ $$ = $1; }
	| statement statement_list
							{ $$ = operate(T_SEPARATOR, 2, $1, $2); }
	;

statement:
	expression T_SEPARATOR	{ $$ = $1; }
	| T_ECHO string T_SEPARATOR	
							{ $$ = operate(T_ECHO, 1, $2); }
	| T_ECHO expression T_SEPARATOR	
							{ $$ = operate(T_ECHO, 1, $2); }
	| '$' string '=' expression T_SEPARATOR
							{ $$ = operate('$', 2, $2, $4); }
	| T_IF expression T_SEPARATOR statement_list T_ENDIF T_SEPARATOR
							{ $$ = operate(T_IF, 2, $2, $4); }
	| T_WHILE expression T_SEPARATOR statement_list T_ENDWHILE T_SEPARATOR
							{ $$ = operate(T_WHILE, 2, $2, $4); }
	;

expression:
	T_INTEGER				{ $$ = get_constant($1); }
	| '(' expression ')'	{ $$ = $2; }
	| expression '=' expression 
							{ $$ = operate('=', 2, $1, $3); }
	| expression '>' expression 
							{ $$ = operate('>', 2, $1, $3); }
	| expression '<' expression 
							{ $$ = operate('<', 2, $1, $3); }
	| expression T_GE expression 
							{ $$ = operate(T_GE, 2, $1, $3); }
	| expression T_LE expression 
							{ $$ = operate(T_LE, 2, $1, $3); }
	| expression T_NE expression 
							{ $$ = operate(T_NE, 2, $1, $3); }
	| expression '+' expression 
							{ $$ = operate('+', 2, $1, $3); }
	| expression '-' expression 
							{ $$ = operate('-', 2, $1, $3); }
	| expression '*' expression 
							{ $$ = operate('*', 2, $1, $3); }
	| expression '/' expression 
							{ $$ = operate('/', 2, $1, $3); }
	;

string:
	T_STRING				{ $$ = get_identifier($1); }
	;
%%
void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
}
int main(void) {
	yyparse();
	return 0;
}
