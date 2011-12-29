%{
/*
 * eh.y
 * Jelle Zijlstra, December 2011
 *
 * Yacc grammar specification for the EH scripting language. The first versions
 * of this code were inspired by Tom Niemann's "A Compact Guide to Lex & Yacc", 
 * available at http://epaperpress.com/lexandyacc/
 */
#include "eh_error.h"
extern FILE *yyin;
#define YYERROR_VERBOSE
extern int yylineno;
%}
%union {
	char *sValue;
	int iValue;
	type_enum tValue;
	visibility_enum vValue;
	bool bValue;
	struct ehretval_t *ehNode;
	accessor_enum aValue;
	magicvar_enum mValue;
};
%token <iValue> T_INTEGER
%token <tValue> T_TYPE
%token <bValue> T_BOOL
%token T_IF
%token T_ELSE
%token T_ENDIF
%token T_WHILE
%token T_ENDWHILE
%token T_FOR
%token T_ENDFOR
%token T_BREAK
%token T_CONTINUE
%token T_FUNC
%token T_ENDFUNC
%token T_RET
%token T_ECHO
%token T_SEPARATOR
%token T_SET
%token T_CALL
%token T_NULL
%token T_CLASS
%token T_ENDCLASS
%token T_THIS
%token T_GLOBAL
%token T_LVALUE
%token T_LVALUE_OBJECT
%token <vValue> T_VISIBILITY
%token T_ARRAYMEMBER
%token T_EXPRESSION
%token T_DOUBLEARROW
%token <sValue> T_VARIABLE
%token <mValue> T_MAGICVAR
%token <sValue> T_STRING
%left ','
%left T_AND T_OR T_XOR
%left ':'
%left '|' '^' '&'
%left '+' '-'
%left '=' '>' '<' T_GE T_LE T_NE T_SE
%left '*' '/' '%'
%nonassoc T_PLUSPLUS T_MINMIN
%right <aValue> T_ACCESSOR
%nonassoc '$' T_REFERENCE '~' '!' T_NEGATIVE T_COUNT
%nonassoc T_NEW
%nonassoc '[' ']'
%nonassoc '(' ')'

%type<ehNode> statement expression statement_list bareword arglist arg parglist arraylist arraymember arraymemberwrap expressionwrap classlist classmember lvalue parg
%%
program:
	statement_list			{
								//print_tree($1, 0);
								eh_init();
								ehretval_t ret = execute($1, NULL);
								free_node($1);
								eh_exit();
								// use exit value if possible
								if(ret.type == int_e)
									exit(ret.intval);
								else
									exit(0);
							}
	| /* NULL */			{
								fprintf(stderr, "No input\n");
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
	| T_ECHO expression T_SEPARATOR
							{ $$ = operate(T_ECHO, 1, $2); }
	| T_SET lvalue '=' expression T_SEPARATOR
							{ $$ = operate(T_SET, 2, $2, $4); }
	| T_SET lvalue T_PLUSPLUS T_SEPARATOR
							{ $$ = operate(T_PLUSPLUS, 1, $2); }
	| T_SET lvalue T_MINMIN T_SEPARATOR
							{ $$ = operate(T_MINMIN, 1, $2); }
	| T_IF expression T_SEPARATOR statement_list T_ENDIF T_SEPARATOR
							{ $$ = operate(T_IF, 2, $2, $4); }
	| T_IF expression T_SEPARATOR statement_list T_ELSE T_SEPARATOR statement_list T_ENDIF T_SEPARATOR
							{ $$ = operate(T_IF, 3, $2, $4, $7); }
	| T_WHILE expression T_SEPARATOR statement_list T_ENDWHILE T_SEPARATOR
							{ $$ = operate(T_WHILE, 2, $2, $4); }
	| T_FOR expression T_SEPARATOR statement_list T_ENDFOR T_SEPARATOR
							{ $$ = operate(T_FOR, 2, $2, $4); }
	| T_FOR expression T_COUNT bareword T_SEPARATOR statement_list T_ENDFOR T_SEPARATOR
							{ $$ = operate(T_FOR, 3, $2, $4, $6); }
	| T_CALL expression T_SEPARATOR
							{ $$ = operate(T_CALL, 1, $2); }
	| T_FUNC bareword ':' parglist T_SEPARATOR statement_list T_ENDFUNC T_SEPARATOR
							{ $$ = operate(T_FUNC, 3, $2, $4, $6); }
	| T_RET expression T_SEPARATOR
							{ $$ = operate(T_RET, 1, $2); }
	| T_CLASS bareword T_SEPARATOR classlist T_ENDCLASS T_SEPARATOR
							{ $$ = operate(T_CLASS, 2, $2, $4); }
	| T_GLOBAL bareword	T_SEPARATOR
							{ $$ = operate(T_GLOBAL, 1, $2); }
	| T_CONTINUE T_SEPARATOR
							{ $$ = operate(T_CONTINUE, 0); }
	| T_CONTINUE expression T_SEPARATOR
							{ $$ = operate(T_CONTINUE, 1, $2); }
	| T_BREAK T_SEPARATOR	{ $$ = operate(T_BREAK, 0); }
	| T_BREAK expression T_SEPARATOR
							{ $$ = operate(T_BREAK, 1, $2); }
	;

expression:
	T_INTEGER				{ $$ = get_int($1); }
	| T_STRING				{ $$ = get_string($1); }
	| T_NULL				{ $$ = get_null(); }
	| T_BOOL				{ $$ = get_bool($1); }
	| bareword				{ $$ = $1; }
	| '(' expression ')'	{ $$ = $2; }
	| '~' expression		{ $$ = operate('~', 1, $2); }
	| '!' expression		{ $$ = operate('!', 1, $2); }
	| '-' expression %prec T_NEGATIVE
							{ $$ = operate(T_NEGATIVE, 1, $2); }
	| expression T_ACCESSOR expression
							{ $$ = operate(T_ACCESSOR, 3, $1, get_accessor($2), $3); }
	| '$' lvalue			{ $$ = operate('$', 1, $2); }
	| '&' lvalue %prec T_REFERENCE
							{ $$ = operate(T_REFERENCE, 1, $2); }
	| '@' T_TYPE expression	{ $$ = operate('@', 2, get_type($2), $3); }
	| expression ':' arglist
							{ $$ = operate(':', 2, $1, $3); }
	| expression '=' expression
							{ $$ = operate('=', 2, $1, $3); }
	| expression '>' expression
							{ $$ = operate('>', 2, $1, $3); }
	| expression '<' expression
							{ $$ = operate('<', 2, $1, $3); }
	| expression T_SE expression
							{ $$ = operate(T_SE, 2, $1, $3); }
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
	| expression '%' expression
							{ $$ = operate('%', 2, $1, $3); }
	| expression '^' expression
							{ $$ = operate('^', 2, $1, $3); }
	| expression '|' expression
							{ $$ = operate('|', 2, $1, $3); }
	| expression '&' expression
							{ $$ = operate('&', 2, $1, $3); }
	| expression T_AND expression
							{ $$ = operate(T_AND, 2, $1, $3); }
	| expression T_OR expression
							{ $$ = operate(T_OR, 2, $1, $3); }
	| expression T_XOR expression
							{ $$ = operate(T_XOR, 2, $1, $3); }
	| '[' arraylist ']'		{ $$ = operate('[', 1, $2); }
	| T_COUNT expression	{ $$ = operate(T_COUNT, 1, $2); }
	| T_NEW bareword		{ $$ = operate(T_NEW, 1, $2); }
	;

bareword:
	T_VARIABLE				{ $$ = get_string($1); }
	;

arraylist:
	arraylist arraymemberwrap
							{ $$ = operate(',', 2, $1, $2); }
	| /* NULL */			{ $$ = operate(',', 0); }
	;

arraymemberwrap:
	arraymember				{ $$ = $1; }
	| arraymember ','		{ $$ = $1; }
	;

arraymember:
	expressionwrap T_DOUBLEARROW expressionwrap
							{ $$ = operate(T_ARRAYMEMBER, 2, $1, $3); }
	| expressionwrap		{ $$ = operate(T_ARRAYMEMBER, 1, $1); }
	;

arglist:
	arglist arg				{ $$ = operate(',', 2, $1, $2); }
	| /* NULL */			{ $$ = operate(',', 0); }
	;

arg:
	expression				{ $$ = $1; }
	| expression ','		{ $$ = $1; }
	;

parglist:
	parglist parg			{ $$ = operate(',', 2, $1, $2); }
	| /* NULL */			{ $$ = operate(',', 0); }
	;

parg:
	bareword				{ $$ = $1; }
	| bareword ','			{ $$ = $1; }

expressionwrap:
	expression				{ $$ = operate(T_EXPRESSION, 1, $1); }
	;

classlist:
	classmember T_SEPARATOR classlist
							{ $$ = operate(',', 2, $1, $3); }
	| /* NULL */			{ $$ = 0; }
	;

lvalue:
	bareword				{ $$ = operate(T_LVALUE, 1, $1); }
	| bareword T_ACCESSOR expression
							{ $$ = operate(T_LVALUE, 3, $1, get_accessor($2), $3); }
	| T_MAGICVAR T_ACCESSOR expression
							{ $$ = operate(T_LVALUE, 3, get_magicvar($1), get_accessor($2), $3); }
	;

classmember:
	T_VISIBILITY bareword
							{ /* property declaration */
								$$ = operate(T_VISIBILITY, 2, get_visibility($1), $2);
							}
	| T_VISIBILITY bareword '=' expression
							{ 
								$$ = operate(T_VISIBILITY, 3, get_visibility($1), $2, $4); 
							}
	| T_VISIBILITY bareword ':' parglist T_SEPARATOR statement_list T_ENDFUNC
							{ 
								$$ = operate(T_VISIBILITY, 4, get_visibility($1), $2, $4, $6); 
							}
	;
%%
void yyerror(char *s) {
	eh_error_line(yylineno, s);
}
int main(int argc, char **argv) {
	if(argc < 2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		eh_error(NULL, efatal_e);
	}
	FILE *infile = fopen(argv[1], "r");
	if(!infile)
		eh_error("Unable to open input file", efatal_e);
	eh_setarg(argc, argv);
	// set input
	yyin = infile;
	yyparse();
	return 0;
}
