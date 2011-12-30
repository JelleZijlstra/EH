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
%token T_GIVEN
%token T_END
%token T_SWITCH
%token T_ENDSWITCH
%token T_CASE
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
%token T_GLOBAL
%token T_LVALUE
%token <vValue> T_ATTRIBUTE
%token T_ARRAYMEMBER
%token T_EXPRESSION
%token T_DOUBLEARROW
%token T_COMMAND T_SHORTPARA T_LONGPARA
%token <sValue> T_VARIABLE
%token <mValue> T_MAGICVAR
%token <sValue> T_STRING
%left T_LOWPREC /* Used to prevent S/R conflicts */
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

%type<ehNode> statement expression statement_list bareword arglist arg parglist arraylist arraymember arraymemberwrap expressionwrap classlist classmember lvalue parg attributelist caselist acase exprcaselist exprcase command paralist para simple_expr line_expr
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
	line_expr T_SEPARATOR	{ $$ = $1; }
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
	| T_SWITCH expression T_SEPARATOR caselist T_ENDSWITCH T_SEPARATOR
							{ $$ = operate(T_SWITCH, 2, $2, $4); }
	| command T_SEPARATOR	{ $$ = $1; }
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
	| T_FUNC ':' parglist T_SEPARATOR statement_list T_ENDFUNC
							{ $$ = operate(T_FUNC, 2, $3, $5); }
	| T_GIVEN expression T_SEPARATOR exprcaselist T_END
							{ $$ = operate(T_GIVEN, 2, $2, $4); }
	;

simple_expr:
	T_INTEGER				{ $$ = get_int($1); }
	| T_STRING				{ $$ = get_string($1); }
	| T_NULL				{ $$ = get_null(); }
	| T_BOOL				{ $$ = get_bool($1); }
	| bareword				{ $$ = $1; }
	| '(' expression ')'	{ $$ = $2; }
	| '~' simple_expr		{ $$ = operate('~', 1, $2); }
	| '!' simple_expr		{ $$ = operate('!', 1, $2); }
	| simple_expr T_ACCESSOR simple_expr
							{ $$ = operate(T_ACCESSOR, 3, $1, get_accessor($2), $3); }
	| '$' lvalue			{ $$ = operate('$', 1, $2); }
	| '@' T_TYPE simple_expr
							{ $$ = operate('@', 2, get_type($2), $3); }
	| simple_expr ':' arglist
							{ $$ = operate(':', 2, $1, $3); }
	| simple_expr '=' simple_expr
							{ $$ = operate('=', 2, $1, $3); }
	| simple_expr '>' simple_expr
							{ $$ = operate('>', 2, $1, $3); }
	| simple_expr '<' simple_expr
							{ $$ = operate('<', 2, $1, $3); }
	| simple_expr T_SE simple_expr
							{ $$ = operate(T_SE, 2, $1, $3); }
	| simple_expr T_GE simple_expr
							{ $$ = operate(T_GE, 2, $1, $3); }
	| simple_expr T_LE simple_expr
							{ $$ = operate(T_LE, 2, $1, $3); }
	| simple_expr T_NE simple_expr
							{ $$ = operate(T_NE, 2, $1, $3); }
	| simple_expr '+' simple_expr
							{ $$ = operate('+', 2, $1, $3); }
	| simple_expr '*' simple_expr
							{ $$ = operate('*', 2, $1, $3); }
	| simple_expr '/' simple_expr
							{ $$ = operate('/', 2, $1, $3); }
	| simple_expr '%' simple_expr
							{ $$ = operate('%', 2, $1, $3); }
	| simple_expr '^' simple_expr
							{ $$ = operate('^', 2, $1, $3); }
	| simple_expr '|' simple_expr
							{ $$ = operate('|', 2, $1, $3); }
	| simple_expr '&' simple_expr
							{ $$ = operate('&', 2, $1, $3); }
	| simple_expr T_AND simple_expr
							{ $$ = operate(T_AND, 2, $1, $3); }
	| simple_expr T_OR simple_expr
							{ $$ = operate(T_OR, 2, $1, $3); }
	| simple_expr T_XOR simple_expr
							{ $$ = operate(T_XOR, 2, $1, $3); }
	| '[' arraylist ']'		{ $$ = operate('[', 1, $2); }
	| T_COUNT simple_expr	{ $$ = operate(T_COUNT, 1, $2); }
	| T_NEW bareword		{ $$ = operate(T_NEW, 1, $2); }
	;

line_expr: 
	/* need to separate expressions beginning with a bareword from commands */
	T_INTEGER				{ $$ = get_int($1); }
	| T_STRING				{ $$ = get_string($1); }
	| T_NULL				{ $$ = get_null(); }
	| T_BOOL				{ $$ = get_bool($1); }
	| '(' expression ')'	{ $$ = $2; }
	| '~' expression		{ $$ = operate('~', 1, $2); }
	| '!' expression		{ $$ = operate('!', 1, $2); }
	| '-' expression %prec T_NEGATIVE
							{ $$ = operate(T_NEGATIVE, 1, $2); }
	| line_expr T_ACCESSOR expression
							{ $$ = operate(T_ACCESSOR, 3, $1, get_accessor($2), $3); }
	| '$' lvalue			{ $$ = operate('$', 1, $2); }
	| '&' lvalue %prec T_REFERENCE
							{ $$ = operate(T_REFERENCE, 1, $2); }
	| '@' T_TYPE expression	{ $$ = operate('@', 2, get_type($2), $3); }
	| bareword ':' arglist
							{ $$ = operate(':', 2, $1, $3); }
	| '$' lvalue ':' arglist
							{ $$ = operate(':', 2,
								operate('$', 1, $2),
								$4);
							}
	| line_expr '=' expression
							{ $$ = operate('=', 2, $1, $3); }
	| line_expr '>' expression
							{ $$ = operate('>', 2, $1, $3); }
	| line_expr '<' expression
							{ $$ = operate('<', 2, $1, $3); }
	| line_expr T_SE expression
							{ $$ = operate(T_SE, 2, $1, $3); }
	| line_expr T_GE expression
							{ $$ = operate(T_GE, 2, $1, $3); }
	| line_expr T_LE expression
							{ $$ = operate(T_LE, 2, $1, $3); }
	| line_expr T_NE expression
							{ $$ = operate(T_NE, 2, $1, $3); }
	| line_expr '+' expression
							{ $$ = operate('+', 2, $1, $3); }
	| line_expr '-' expression
							{ $$ = operate('-', 2, $1, $3); }
	| line_expr '*' expression
							{ $$ = operate('*', 2, $1, $3); }
	| line_expr '/' expression
							{ $$ = operate('/', 2, $1, $3); }
	| line_expr '%' expression
							{ $$ = operate('%', 2, $1, $3); }
	| line_expr '^' expression
							{ $$ = operate('^', 2, $1, $3); }
	| line_expr '|' expression
							{ $$ = operate('|', 2, $1, $3); }
	| line_expr '&' expression
							{ $$ = operate('&', 2, $1, $3); }
	| line_expr T_AND expression
							{ $$ = operate(T_AND, 2, $1, $3); }
	| line_expr T_OR expression
							{ $$ = operate(T_OR, 2, $1, $3); }
	| line_expr T_XOR expression
							{ $$ = operate(T_XOR, 2, $1, $3); }
	| '[' arraylist ']'		{ $$ = operate('[', 1, $2); }
	| T_COUNT expression	{ $$ = operate(T_COUNT, 1, $2); }
	| T_NEW bareword		{ $$ = operate(T_NEW, 1, $2); }
	| T_FUNC ':' parglist T_SEPARATOR statement_list T_ENDFUNC
							{ $$ = operate(T_FUNC, 2, $3, $5); }
	| T_GIVEN expression T_SEPARATOR exprcaselist T_END
							{ $$ = operate(T_GIVEN, 2, $2, $4); }
	;

command:
	bareword paralist		{ $$ = operate(T_COMMAND, 2, $1, $2); }

paralist:
	paralist para			{ $$ = operate(',', 2, $1, $2); }
	| /* NULL */			{ $$ = operate(',', 0); }
	;

para:
	T_STRING				{ $$ = get_string($1); }
	| bareword				{ $$ = $1; }
	| T_MINMIN bareword '=' simple_expr
							{ $$ = operate(T_LONGPARA, 2, $2, $4); }
	| T_MINMIN T_STRING '=' simple_expr
							{ $$ = operate(T_LONGPARA, 2, get_string($2), $4); }
	| '-' bareword			{ $$ = operate(T_SHORTPARA, 1, $2); }
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
	expression %prec T_LOWPREC				{ $$ = $1; }
	| expression ','		{ $$ = $1; }
	;

parglist:
	parglist parg			{ $$ = operate(',', 2, $1, $2); }
	| /* NULL */			{ $$ = operate(',', 0); }
	;

parg:
	bareword				{ $$ = $1; }
	| bareword ','			{ $$ = $1; }

caselist:
	caselist acase			{ $$ = operate(',', 2, $1, $2); }
	| /* NULL */			{ $$ = operate(',', 0); }
	;

acase:
	T_CASE expression T_SEPARATOR statement_list
							{ $$ = operate(T_CASE, 2, $2, $4); }

exprcaselist:
	exprcaselist exprcase	{ $$ = operate(',', 2, $1, $2); }
	| /* NULL */			{ $$ = operate(',', 0); }
	;

exprcase:
	T_CASE expression T_SEPARATOR expression T_SEPARATOR
							{ $$ = operate(T_CASE, 2, $2, $4); }

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

classmember: /* , is used as the operator token for those, because none is really needed and , is the generic null token */
	attributelist bareword	{ /* property declaration */
								$$ = operate(',', 2, $1, $2);
							}
	| attributelist bareword '=' expression
							{ $$ = operate(',', 3, $1, $2, $4); }
	| attributelist bareword ':' parglist T_SEPARATOR statement_list T_ENDFUNC
							{ $$ = operate(',', 4, $1, $2, $4, $6); }
	;

attributelist:
	attributelist T_ATTRIBUTE
							{ $$ = operate(T_ATTRIBUTE, 2, $1, get_attribute($2)); }
	| /* NULL */			{ $$ = operate(T_ATTRIBUTE, 0); }
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
