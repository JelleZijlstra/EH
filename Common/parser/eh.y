/* This code was inspired by Tom Niemann's "A Compact Guide to Lex & Yacc", available at http://epaperpress.com/lexandyacc/ */
%{
#include "eh.h"

%}
%union {
	char *sValue;
	int iValue;
	struct ehnode_t *ehNode;
};
%token <iValue> T_INTEGER
%token T_IF
%token T_ENDIF
%token T_WHILE
%token T_ENDWHILE
%token T_FUNC
%token T_ENDFUNC
%token T_RET
%token T_ECHO
%token T_SEPARATOR
%token T_SET
%token T_CALL
%token <sValue> T_VARIABLE
%token <sValue> T_STRING
%nonassoc ':'
%left ','
%left '=' '>' '<' T_GE T_LE T_NE
%left '+' '-'
%left '*' '/'
%nonassoc '(' ')'

%type<ehNode> statement expression statement_list string bareword arglist parglist
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
	| T_SET bareword '=' expression T_SEPARATOR
							{ $$ = operate(T_SET, 2, $2, $4); }
	| T_IF expression T_SEPARATOR statement_list T_ENDIF T_SEPARATOR
							{ $$ = operate(T_IF, 2, $2, $4); }
	| T_WHILE expression T_SEPARATOR statement_list T_ENDWHILE T_SEPARATOR
							{ $$ = operate(T_WHILE, 2, $2, $4); }
	| T_CALL expression T_SEPARATOR	
							{ $$ = operate(T_CALL, 1, $2); }
	| T_FUNC bareword ':' parglist T_SEPARATOR statement_list T_ENDFUNC T_SEPARATOR
							{ $$ = operate(T_FUNC, 3, $2, $4, $6); }
	| T_FUNC bareword ':' T_SEPARATOR statement_list T_ENDFUNC T_SEPARATOR
							{ $$ = operate(T_FUNC, 2, $2, $5); }
	| T_RET expression T_SEPARATOR
							{ $$ = operate(T_RET, 1, $2); }
	;

expression:
	T_INTEGER				{ $$ = get_constant($1); }
	| '(' expression ')'	{ $$ = $2; }
	| '$' bareword			{ $$ = operate('$', 1, $2); }
	| bareword ':' arglist	{ $$ = operate(':', 2, $1, $3); }
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

bareword:
	T_VARIABLE				{ $$ = get_identifier($1); }
	;

arglist:
	expression				{ $$ = $1; }
	| expression ',' arglist
							{ $$ = operate(',', 2, $1, $3); }
	| /* NULL */			{ $$ = 0; }
	;

parglist:
	bareword				{ $$ = $1; }
	| bareword ',' parglist
							{ $$ = operate(',', 2, $1, $3); }
	| /* NULL */			{ $$ = 0; }
	;
%%
void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
}
int main(void) {
	yyparse();
	return 0;
}
