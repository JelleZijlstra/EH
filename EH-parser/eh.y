%{
/*
 * eh.y
 * Jelle Zijlstra, December 2011
 *
 * Yacc grammar specification for the EH scripting language. The first versions
 * of this code were inspired by Tom Niemann's "A Compact Guide to Lex & Yacc", 
 * available at http://epaperpress.com/lexandyacc/
 */
#include "eh.h"
#include "y.tab.h"
extern FILE *yyin;
#define YYERROR_VERBOSE
extern int yylineno;
void eh_outer_exit(int exitval);
bool is_interactive = false;
struct yy_buffer_state *yy_scan_string ( const char *str );
%}
%union {
	char *sValue;
	int iValue;
	type_enum tValue;
	attribute_enum vValue;
	bool bValue;
	ehretval_t *ehNode;
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

%type<ehNode> statement expression statement_list bareword arglist arg parglist arraylist arraymember arraymemberwrap expressionwrap classlist classmember lvalue parg attributelist caselist acase exprcaselist exprcase command paralist para simple_expr line_expr global_list
%%
global_list: /* NULL */		{ }
	| global_list statement	{ 
								ehretval_t ret = eh_execute($2, NULL);
								if(returning) 
									eh_outer_exit(ret.intval);
							}
	;

statement_list:
	/* NULL */				{ $$ = eh_addnode(T_SEPARATOR, 0); }
	| statement statement_list
							{ $$ = eh_addnode(T_SEPARATOR, 2, $1, $2); }
	;

statement:
	T_SEPARATOR				{ $$ = 0; }
	| line_expr T_SEPARATOR	{ $$ = $1; }
	| T_ECHO expression T_SEPARATOR
							{ $$ = eh_addnode(T_ECHO, 1, $2); }
	| T_SET lvalue '=' expression T_SEPARATOR
							{ $$ = eh_addnode(T_SET, 2, $2, $4); }
	| T_SET lvalue T_PLUSPLUS T_SEPARATOR
							{ $$ = eh_addnode(T_PLUSPLUS, 1, $2); }
	| T_SET lvalue T_MINMIN T_SEPARATOR
							{ $$ = eh_addnode(T_MINMIN, 1, $2); }
	| T_IF expression T_SEPARATOR statement_list T_ENDIF T_SEPARATOR
							{ $$ = eh_addnode(T_IF, 2, $2, $4); }
	| T_IF expression T_SEPARATOR statement_list T_ELSE T_SEPARATOR statement_list T_ENDIF T_SEPARATOR
							{ $$ = eh_addnode(T_IF, 3, $2, $4, $7); }
	| T_WHILE expression T_SEPARATOR statement_list T_ENDWHILE T_SEPARATOR
							{ $$ = eh_addnode(T_WHILE, 2, $2, $4); }
	| T_FOR expression T_SEPARATOR statement_list T_ENDFOR T_SEPARATOR
							{ $$ = eh_addnode(T_FOR, 2, $2, $4); }
	| T_FOR expression T_COUNT bareword T_SEPARATOR statement_list T_ENDFOR T_SEPARATOR
							{ $$ = eh_addnode(T_FOR, 3, $2, $4, $6); }
	| T_CALL expression T_SEPARATOR
							{ $$ = eh_addnode(T_CALL, 1, $2); }
	| T_FUNC bareword ':' parglist T_SEPARATOR statement_list T_ENDFUNC T_SEPARATOR
							{ $$ = eh_addnode(T_FUNC, 3, $2, $4, $6); }
	| T_RET expression T_SEPARATOR
							{ $$ = eh_addnode(T_RET, 1, $2); }
	| T_CLASS bareword T_SEPARATOR classlist T_ENDCLASS T_SEPARATOR
							{ $$ = eh_addnode(T_CLASS, 2, $2, $4); }
	| T_GLOBAL bareword	T_SEPARATOR
							{ $$ = eh_addnode(T_GLOBAL, 1, $2); }
	| T_CONTINUE T_SEPARATOR
							{ $$ = eh_addnode(T_CONTINUE, 0); }
	| T_CONTINUE expression T_SEPARATOR
							{ $$ = eh_addnode(T_CONTINUE, 1, $2); }
	| T_BREAK T_SEPARATOR	{ $$ = eh_addnode(T_BREAK, 0); }
	| T_BREAK expression T_SEPARATOR
							{ $$ = eh_addnode(T_BREAK, 1, $2); }
	| T_SWITCH expression T_SEPARATOR caselist T_ENDSWITCH T_SEPARATOR
							{ $$ = eh_addnode(T_SWITCH, 2, $2, $4); }
	| command T_SEPARATOR	{ $$ = $1; }
	;

expression:
	T_INTEGER				{ $$ = eh_get_int($1); }
	| T_STRING				{ $$ = eh_get_string($1); }
	| T_NULL				{ $$ = eh_get_null(); }
	| T_BOOL				{ $$ = eh_get_bool($1); }
	| bareword				{ $$ = $1; }
	| '(' expression ')'	{ $$ = $2; }
	| '~' expression		{ $$ = eh_addnode('~', 1, $2); }
	| '!' expression		{ $$ = eh_addnode('!', 1, $2); }
	| '-' expression %prec T_NEGATIVE
							{ $$ = eh_addnode(T_NEGATIVE, 1, $2); }
	| expression T_ACCESSOR expression
							{ $$ = eh_addnode(T_ACCESSOR, 3, $1, eh_get_accessor($2), $3); }
	| '$' lvalue			{ $$ = eh_addnode('$', 1, $2); }
	| '&' lvalue %prec T_REFERENCE
							{ $$ = eh_addnode(T_REFERENCE, 1, $2); }
	| '@' T_TYPE expression	{ $$ = eh_addnode('@', 2, eh_get_type($2), $3); }
	| expression ':' arglist
							{ $$ = eh_addnode(':', 2, $1, $3); }
	| expression '=' expression
							{ $$ = eh_addnode('=', 2, $1, $3); }
	| expression '>' expression
							{ $$ = eh_addnode('>', 2, $1, $3); }
	| expression '<' expression
							{ $$ = eh_addnode('<', 2, $1, $3); }
	| expression T_SE expression
							{ $$ = eh_addnode(T_SE, 2, $1, $3); }
	| expression T_GE expression
							{ $$ = eh_addnode(T_GE, 2, $1, $3); }
	| expression T_LE expression
							{ $$ = eh_addnode(T_LE, 2, $1, $3); }
	| expression T_NE expression
							{ $$ = eh_addnode(T_NE, 2, $1, $3); }
	| expression '+' expression
							{ $$ = eh_addnode('+', 2, $1, $3); }
	| expression '-' expression
							{ $$ = eh_addnode('-', 2, $1, $3); }
	| expression '*' expression
							{ $$ = eh_addnode('*', 2, $1, $3); }
	| expression '/' expression
							{ $$ = eh_addnode('/', 2, $1, $3); }
	| expression '%' expression
							{ $$ = eh_addnode('%', 2, $1, $3); }
	| expression '^' expression
							{ $$ = eh_addnode('^', 2, $1, $3); }
	| expression '|' expression
							{ $$ = eh_addnode('|', 2, $1, $3); }
	| expression '&' expression
							{ $$ = eh_addnode('&', 2, $1, $3); }
	| expression T_AND expression
							{ $$ = eh_addnode(T_AND, 2, $1, $3); }
	| expression T_OR expression
							{ $$ = eh_addnode(T_OR, 2, $1, $3); }
	| expression T_XOR expression
							{ $$ = eh_addnode(T_XOR, 2, $1, $3); }
	| '[' arraylist ']'		{ $$ = eh_addnode('[', 1, $2); }
	| T_COUNT expression	{ $$ = eh_addnode(T_COUNT, 1, $2); }
	| T_NEW bareword		{ $$ = eh_addnode(T_NEW, 1, $2); }
	| T_FUNC ':' parglist T_SEPARATOR statement_list T_ENDFUNC
							{ $$ = eh_addnode(T_FUNC, 2, $3, $5); }
	| T_GIVEN expression T_SEPARATOR exprcaselist T_END
							{ $$ = eh_addnode(T_GIVEN, 2, $2, $4); }
	;

simple_expr:
	T_INTEGER				{ $$ = eh_get_int($1); }
	| T_STRING				{ $$ = eh_get_string($1); }
	| T_NULL				{ $$ = eh_get_null(); }
	| T_BOOL				{ $$ = eh_get_bool($1); }
	| bareword				{ $$ = $1; }
	| '(' expression ')'	{ $$ = $2; }
	| '~' simple_expr		{ $$ = eh_addnode('~', 1, $2); }
	| '!' simple_expr		{ $$ = eh_addnode('!', 1, $2); }
	| simple_expr T_ACCESSOR simple_expr
							{ $$ = eh_addnode(T_ACCESSOR, 3, $1, eh_get_accessor($2), $3); }
	| '$' lvalue			{ $$ = eh_addnode('$', 1, $2); }
	| '@' T_TYPE simple_expr
							{ $$ = eh_addnode('@', 2, eh_get_type($2), $3); }
	| simple_expr ':' arglist
							{ $$ = eh_addnode(':', 2, $1, $3); }
	| simple_expr '=' simple_expr
							{ $$ = eh_addnode('=', 2, $1, $3); }
	| simple_expr '>' simple_expr
							{ $$ = eh_addnode('>', 2, $1, $3); }
	| simple_expr '<' simple_expr
							{ $$ = eh_addnode('<', 2, $1, $3); }
	| simple_expr T_SE simple_expr
							{ $$ = eh_addnode(T_SE, 2, $1, $3); }
	| simple_expr T_GE simple_expr
							{ $$ = eh_addnode(T_GE, 2, $1, $3); }
	| simple_expr T_LE simple_expr
							{ $$ = eh_addnode(T_LE, 2, $1, $3); }
	| simple_expr T_NE simple_expr
							{ $$ = eh_addnode(T_NE, 2, $1, $3); }
	| simple_expr '+' simple_expr
							{ $$ = eh_addnode('+', 2, $1, $3); }
	| simple_expr '*' simple_expr
							{ $$ = eh_addnode('*', 2, $1, $3); }
	| simple_expr '/' simple_expr
							{ $$ = eh_addnode('/', 2, $1, $3); }
	| simple_expr '%' simple_expr
							{ $$ = eh_addnode('%', 2, $1, $3); }
	| simple_expr '^' simple_expr
							{ $$ = eh_addnode('^', 2, $1, $3); }
	| simple_expr '|' simple_expr
							{ $$ = eh_addnode('|', 2, $1, $3); }
	| simple_expr '&' simple_expr
							{ $$ = eh_addnode('&', 2, $1, $3); }
	| simple_expr T_AND simple_expr
							{ $$ = eh_addnode(T_AND, 2, $1, $3); }
	| simple_expr T_OR simple_expr
							{ $$ = eh_addnode(T_OR, 2, $1, $3); }
	| simple_expr T_XOR simple_expr
							{ $$ = eh_addnode(T_XOR, 2, $1, $3); }
	| '[' arraylist ']'		{ $$ = eh_addnode('[', 1, $2); }
	| T_COUNT simple_expr	{ $$ = eh_addnode(T_COUNT, 1, $2); }
	| T_NEW bareword		{ $$ = eh_addnode(T_NEW, 1, $2); }
	;

line_expr: 
	/* need to separate expressions beginning with a bareword from commands */
	T_INTEGER				{ $$ = eh_get_int($1); }
	| T_STRING				{ $$ = eh_get_string($1); }
	| T_NULL				{ $$ = eh_get_null(); }
	| T_BOOL				{ $$ = eh_get_bool($1); }
	| '(' expression ')'	{ $$ = $2; }
	| '~' expression		{ $$ = eh_addnode('~', 1, $2); }
	| '!' expression		{ $$ = eh_addnode('!', 1, $2); }
	| '-' expression %prec T_NEGATIVE
							{ $$ = eh_addnode(T_NEGATIVE, 1, $2); }
	| line_expr T_ACCESSOR expression
							{ $$ = eh_addnode(T_ACCESSOR, 3, $1, eh_get_accessor($2), $3); }
	| '$' lvalue			{ $$ = eh_addnode('$', 1, $2); }
	| '&' lvalue %prec T_REFERENCE
							{ $$ = eh_addnode(T_REFERENCE, 1, $2); }
	| '@' T_TYPE expression	{ $$ = eh_addnode('@', 2, eh_get_type($2), $3); }
	| bareword ':' arglist
							{ $$ = eh_addnode(':', 2, $1, $3); }
	| '$' lvalue ':' arglist
							{ $$ = eh_addnode(':', 2,
								eh_addnode('$', 1, $2),
								$4);
							}
	| line_expr '=' expression
							{ $$ = eh_addnode('=', 2, $1, $3); }
	| line_expr '>' expression
							{ $$ = eh_addnode('>', 2, $1, $3); }
	| line_expr '<' expression
							{ $$ = eh_addnode('<', 2, $1, $3); }
	| line_expr T_SE expression
							{ $$ = eh_addnode(T_SE, 2, $1, $3); }
	| line_expr T_GE expression
							{ $$ = eh_addnode(T_GE, 2, $1, $3); }
	| line_expr T_LE expression
							{ $$ = eh_addnode(T_LE, 2, $1, $3); }
	| line_expr T_NE expression
							{ $$ = eh_addnode(T_NE, 2, $1, $3); }
	| line_expr '+' expression
							{ $$ = eh_addnode('+', 2, $1, $3); }
	| line_expr '-' expression
							{ $$ = eh_addnode('-', 2, $1, $3); }
	| line_expr '*' expression
							{ $$ = eh_addnode('*', 2, $1, $3); }
	| line_expr '/' expression
							{ $$ = eh_addnode('/', 2, $1, $3); }
	| line_expr '%' expression
							{ $$ = eh_addnode('%', 2, $1, $3); }
	| line_expr '^' expression
							{ $$ = eh_addnode('^', 2, $1, $3); }
	| line_expr '|' expression
							{ $$ = eh_addnode('|', 2, $1, $3); }
	| line_expr '&' expression
							{ $$ = eh_addnode('&', 2, $1, $3); }
	| line_expr T_AND expression
							{ $$ = eh_addnode(T_AND, 2, $1, $3); }
	| line_expr T_OR expression
							{ $$ = eh_addnode(T_OR, 2, $1, $3); }
	| line_expr T_XOR expression
							{ $$ = eh_addnode(T_XOR, 2, $1, $3); }
	| '[' arraylist ']'		{ $$ = eh_addnode('[', 1, $2); }
	| T_COUNT expression	{ $$ = eh_addnode(T_COUNT, 1, $2); }
	| T_NEW bareword		{ $$ = eh_addnode(T_NEW, 1, $2); }
	| T_FUNC ':' parglist T_SEPARATOR statement_list T_ENDFUNC
							{ $$ = eh_addnode(T_FUNC, 2, $3, $5); }
	| T_GIVEN expression T_SEPARATOR exprcaselist T_END
							{ $$ = eh_addnode(T_GIVEN, 2, $2, $4); }
	;

command:
	bareword paralist		{ $$ = eh_addnode(T_COMMAND, 2, $1, $2); }

paralist:
	paralist para			{ $$ = eh_addnode(',', 2, $1, $2); }
	| /* NULL */			{ $$ = eh_addnode(',', 0); }
	;

para:
	T_STRING				{ $$ = eh_get_string($1); }
	| bareword				{ $$ = $1; }
	| T_MINMIN bareword '=' simple_expr
							{ $$ = eh_addnode(T_LONGPARA, 2, $2, $4); }
	| T_MINMIN T_STRING '=' simple_expr
							{ $$ = eh_addnode(T_LONGPARA, 2, eh_get_string($2), $4); }
	| '-' bareword			{ $$ = eh_addnode(T_SHORTPARA, 1, $2); }
	;

bareword:
	T_VARIABLE				{ $$ = eh_get_string($1); }
	;

arraylist:
	arraylist arraymemberwrap
							{ $$ = eh_addnode(',', 2, $1, $2); }
	| /* NULL */			{ $$ = eh_addnode(',', 0); }
	;

arraymemberwrap:
	arraymember				{ $$ = $1; }
	| arraymember ','		{ $$ = $1; }
	;

arraymember:
	expressionwrap T_DOUBLEARROW expressionwrap
							{ $$ = eh_addnode(T_ARRAYMEMBER, 2, $1, $3); }
	| expressionwrap		{ $$ = eh_addnode(T_ARRAYMEMBER, 1, $1); }
	;

arglist:
	arglist arg				{ $$ = eh_addnode(',', 2, $1, $2); }
	| /* NULL */			{ $$ = eh_addnode(',', 0); }
	;

arg:
	expression %prec T_LOWPREC				{ $$ = $1; }
	| expression ','		{ $$ = $1; }
	;

parglist:
	parglist parg			{ $$ = eh_addnode(',', 2, $1, $2); }
	| /* NULL */			{ $$ = eh_addnode(',', 0); }
	;

parg:
	bareword				{ $$ = $1; }
	| bareword ','			{ $$ = $1; }

caselist:
	caselist acase			{ $$ = eh_addnode(',', 2, $1, $2); }
	| /* NULL */			{ $$ = eh_addnode(',', 0); }
	;

acase:
	T_CASE expression T_SEPARATOR statement_list
							{ $$ = eh_addnode(T_CASE, 2, $2, $4); }

exprcaselist:
	exprcaselist exprcase	{ $$ = eh_addnode(',', 2, $1, $2); }
	| /* NULL */			{ $$ = eh_addnode(',', 0); }
	;

exprcase:
	T_CASE expression T_SEPARATOR expression T_SEPARATOR
							{ $$ = eh_addnode(T_CASE, 2, $2, $4); }

expressionwrap:
	expression				{ $$ = eh_addnode(T_EXPRESSION, 1, $1); }
	;

classlist:
	classmember T_SEPARATOR classlist
							{ $$ = eh_addnode(',', 2, $1, $3); }
	| /* NULL */			{ $$ = 0; }
	;

lvalue:
	bareword				{ $$ = eh_addnode(T_LVALUE, 1, $1); }
	| bareword T_ACCESSOR expression
							{ $$ = eh_addnode(T_LVALUE, 3, $1, eh_get_accessor($2), $3); }
	| T_MAGICVAR T_ACCESSOR expression
							{ $$ = eh_addnode(T_LVALUE, 3, eh_get_magicvar($1), eh_get_accessor($2), $3); }
	;

classmember: /* , is used as the operator token for those, because none is really needed and , is the generic null token */
	attributelist bareword	{ /* property declaration */
								$$ = eh_addnode(',', 2, $1, $2);
							}
	| attributelist bareword '=' expression
							{ $$ = eh_addnode(',', 3, $1, $2, $4); }
	| attributelist bareword ':' parglist T_SEPARATOR statement_list T_ENDFUNC
							{ $$ = eh_addnode(',', 4, $1, $2, $4, $6); }
	;

attributelist:
	attributelist T_ATTRIBUTE
							{ $$ = eh_addnode(T_ATTRIBUTE, 2, $1, eh_get_attribute($2)); }
	| /* NULL */			{ $$ = eh_addnode(T_ATTRIBUTE, 0); }
	;
%%
void yyerror(char *s) {
	eh_error_line(yylineno, s);
}
char *eh_getinput(void) {
	char *buf;

	printf("> ");
	buf = (char *) Malloc(512);
	return fgets(buf, 511, stdin);
}
void eh_outer_exit(int exitval) {
	//free_node: something. We should actually be adding stuff to the AST, I suppose.
	eh_exit();
	exit(exitval);
}
int EHI::eh_interactive(void) {
	is_interactive = true;
	yy_scan_string(eh_getinput());
	eh_init();
	yyparse();
	return 0;
}
