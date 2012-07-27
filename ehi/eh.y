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
#include "eh.bison.hpp"
#include "eh_tree.h"
extern FILE *yyin;
EHParser *yyget_extra(void *scanner);
#define YYERROR_VERBOSE
#define YYLEX_PARAM scanner

// can't overload macros
#define ADD_NODE0(opcode) eh_addnode(opcode)
#define ADD_NODE1(opcode, first) eh_addnode(opcode, ehretval_t::make(first))
#define ADD_NODE2(opcode, first, second) eh_addnode(opcode, ehretval_t::make(first), ehretval_t::make(second))
#define ADD_NODE3(opcode, first, second, third) eh_addnode(opcode, ehretval_t::make(first), ehretval_t::make(second), ehretval_t::make(third))
#define ADD_NODE4(opcode, first, second, third, fourth) eh_addnode(opcode, ehretval_t::make(first), ehretval_t::make(second), ehretval_t::make(third), ehretval_t::make(fourth))
%}
%pure-parser
%parse-param { void *scanner }
%union {
	char *sValue;
	int iValue;
	float fValue;
	type_enum tValue;
	attribute_enum vValue;
	bool bValue;
	opnode_t *ehNode;
};
%token <iValue> T_INTEGER
%token <fValue> T_FLOAT
%token <tValue> T_TYPE
%token <bValue> T_BOOL
%token T_IF
%token T_ELSE
%token T_ENDIF
%token T_WHILE
%token T_ENDWHILE
%token T_FOR
%token T_ENDFOR
%token T_AS
%token T_GIVEN
%token T_END
%token T_SWITCH
%token T_DEFAULT
%token T_ENDSWITCH
%token T_CASE
%token T_BREAK
%token T_CONTINUE
%token T_FUNC
%token T_ENDFUNC
%token T_RET
%token T_SEPARATOR
%token T_SET
%token T_NULL
%token T_CLASS
%token T_ENDCLASS
%token T_CLASSMEMBER
%token T_LITERAL
%token T_TRY
%token T_CATCH
%token T_FINALLY
%token T_LVALUE_GET T_LVALUE_SET T_ARROW_SET T_DOT_SET
%token <vValue> T_ATTRIBUTE
%token T_ARRAYMEMBER
%token T_EXPRESSION
%token T_DOUBLEARROW
%token T_COMMAND T_SHORTPARA T_LONGPARA T_REDIRECT
%token T_SHORTFUNCTION
%token <sValue> T_VARIABLE
%token <sValue> T_STRING
%left T_LOWPREC /* Used to prevent S/R conflicts */
%right ':'
%left ','
%right '='
%left T_AND T_OR T_XOR
%left '|' '^' '&'
%left '+' '-'
%left '>' '<' T_GE T_LE T_NE T_SE T_SNE T_EQ T_COMPARE
%left '*' '/' '%'
%nonassoc T_PLUSPLUS T_MINMIN
%right '@'
%left T_ARROW '.'
%nonassoc T_RANGE
%nonassoc '$' '~' '!' T_NEGATIVE T_COUNT
%nonassoc '[' ']' '{' '}'
%nonassoc '(' ')' T_DOLLARPAREN
%nonassoc T_SHORTFUNCTION

%type<ehNode> statement expression statement_list bareword parglist arraylist arraymember arraylist_i anonclasslist anonclassmember anonclasslist_i lvalue_set parg attributelist attributelist_inner caselist acase exprcaselist exprcase command paralist para simple_expr global_list string shortfunc bareword_or_string
%%
program:
	global_list				{ 	// Don't do anything. Destructors below take 
								// care of cleanup.
							}

global_list:
	/* NULL */				{ }
	| statement				{
								EHParser *parser = yyget_extra(scanner);
								ehretval_p statement = ehretval_t::make($1);
								ehretval_p ret;
								try {
									ret = parser->_parent->eh_execute(statement, parser->_parent->global_object);
								} catch(eh_exception &e) {
									std::cerr << "Uncaught exception: " << parser->_parent->to_string(e.content, parser->_parent->global_object)->get_stringval() << std::endl;
								}
								// flush stdout after executing each statement
								fflush(stdout);
								if(parser->_parent->returning) {
									return (ret == NULL) ? 0 : ret->get_intval();
								}
							} global_list {
							}
	;

statement_list:
	/* NULL */				{ $$ = ADD_NODE0(T_SEPARATOR); }
	| statement statement_list
							{ $$ = ADD_NODE2(T_SEPARATOR, $1, $2); }
	;

statement:
	T_SEPARATOR				{ $$ = ADD_NODE0(T_SEPARATOR); }
	| expression T_SEPARATOR	{ $$ = $1; }
	| lvalue_set '=' expression T_SEPARATOR
							{ $$ = ADD_NODE2(T_SET, $1, $3); }
	| T_SET lvalue_set T_PLUSPLUS T_SEPARATOR
							{ $$ = ADD_NODE1(T_PLUSPLUS, $2); }
	| T_SET lvalue_set T_MINMIN T_SEPARATOR
							{ $$ = ADD_NODE1(T_MINMIN, $2); }
		/* Using braces */
	| T_IF expression '{' statement_list '}'
							{ $$ = ADD_NODE2(T_IF, $2, $4); }
	| T_IF expression '{' statement_list '}' T_ELSE '{' statement_list '}'
							{ $$ = ADD_NODE3(T_IF, $2, $4, $8); }
	| T_WHILE expression '{' statement_list '}'
							{ $$ = ADD_NODE2(T_WHILE, $2, $4); }
	| T_FOR expression '{' statement_list '}'
							{ $$ = ADD_NODE2(T_FOR, $2, $4); }
	| T_FOR expression T_COUNT bareword '{' statement_list '}'
							{ $$ = ADD_NODE3(T_FOR, $2, $4, $6); }
	| T_FOR expression T_AS bareword '{' statement_list '}'
							{ $$ = ADD_NODE3(T_AS, $2, $4, $6); }
	| T_FOR expression T_AS bareword T_DOUBLEARROW bareword '{' statement_list '}'
							{ $$ = ADD_NODE4(T_AS, $2, $4, $6, $8); }
	| T_FUNC bareword ':' parglist '{' statement_list '}'
							{ $$ = ADD_NODE2(T_SET,
								ADD_NODE1('$', $2),
								ADD_NODE2(T_FUNC, $4, $6)); }
	| T_SWITCH expression '{' caselist '}'
							{ $$ = ADD_NODE2(T_SWITCH, $2, $4); }
	| T_CLASS bareword '{' statement_list '}'
							{ $$ = ADD_NODE2(T_CLASS, $2, $4); }
		/* Using T_END */
	| T_IF expression T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_IF, $2, $4); }
	| T_IF expression T_SEPARATOR statement_list T_ELSE T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE3(T_IF, $2, $4, $7); }
	| T_WHILE expression T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_WHILE, $2, $4); }
	| T_FOR expression T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_FOR, $2, $4); }
	| T_FOR expression T_COUNT bareword T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE3(T_FOR, $2, $4, $6); }
	| T_FOR expression T_AS bareword T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE3(T_AS, $2, $4, $6); }
	| T_FOR expression T_AS bareword T_DOUBLEARROW bareword T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE4(T_AS, $2, $4, $6, $8); }
	| T_FUNC bareword ':' parglist T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_SET, 
								ADD_NODE1('$', $2),
								ADD_NODE2(T_FUNC, $4, $6)); }
	| T_SWITCH expression T_SEPARATOR caselist T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_SWITCH, $2, $4); }
	| T_CLASS bareword T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_CLASS, $2, $4); }
		/* Endif and endfor */
	| T_IF expression T_SEPARATOR statement_list T_ENDIF T_SEPARATOR
							{ $$ = ADD_NODE2(T_IF, $2, $4); }
	| T_IF expression T_SEPARATOR statement_list T_ELSE T_SEPARATOR statement_list T_ENDIF T_SEPARATOR
							{ $$ = ADD_NODE3(T_IF, $2, $4, $7); }
	| T_WHILE expression T_SEPARATOR statement_list T_ENDWHILE T_SEPARATOR
							{ $$ = ADD_NODE2(T_WHILE, $2, $4); }
	| T_FOR expression T_SEPARATOR statement_list T_ENDFOR T_SEPARATOR
							{ $$ = ADD_NODE2(T_FOR, $2, $4); }
	| T_FOR expression T_COUNT bareword T_SEPARATOR statement_list T_ENDFOR T_SEPARATOR
							{ $$ = ADD_NODE3(T_FOR, $2, $4, $6); }
	| T_FUNC bareword ':' parglist T_SEPARATOR statement_list T_ENDFUNC T_SEPARATOR
							{ $$ = ADD_NODE2(T_SET,
								ADD_NODE1('$', $2),
								ADD_NODE2(T_FUNC, $4, $6)); }
	| T_FUNC bareword ':' parglist T_ARROW expression %prec T_SHORTFUNCTION
							{ $$ = ADD_NODE2(T_SET,
								ADD_NODE1('$', $2),
								ADD_NODE2(T_FUNC, $4, $6)); }
	| T_SWITCH expression T_SEPARATOR caselist T_ENDSWITCH T_SEPARATOR
							{ $$ = ADD_NODE2(T_SWITCH, $2, $4); }
	| T_CLASS bareword T_SEPARATOR statement_list T_ENDCLASS T_SEPARATOR
							{ $$ = ADD_NODE2(T_CLASS, $2, $4); }
		/* Other statements */
	| T_RET expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_RET, $2); }
	| T_CONTINUE T_SEPARATOR
							{ $$ = ADD_NODE0(T_CONTINUE); }
	| T_CONTINUE expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_CONTINUE, $2); }
	| T_BREAK T_SEPARATOR	{ $$ = ADD_NODE0(T_BREAK); }
	| T_BREAK expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_BREAK, $2); }
	| command T_SEPARATOR	{ $$ = $1; }
	| error T_SEPARATOR		{
								yyerrok;
								$$ = ADD_NODE0(T_SEPARATOR);
							}
							/* property declaration */
	| attributelist bareword	
							{ $$ = ADD_NODE2(T_CLASSMEMBER, $1, $2); }
	| attributelist bareword '=' expression
							{ $$ = ADD_NODE3(T_CLASSMEMBER, $1, $2, $4); }
	| attributelist bareword ':' parglist '{' statement_list '}'
							{ $$ = ADD_NODE3(T_CLASSMEMBER, $1, $2, 
									ADD_NODE2(T_FUNC, $4, $6)); }
	| attributelist bareword ':' parglist T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE3(T_CLASSMEMBER, $1, $2, 
									ADD_NODE2(T_FUNC, $4, $6)); }
	| attributelist bareword ':' parglist T_SEPARATOR statement_list T_ENDFUNC
							{ $$ = ADD_NODE3(T_CLASSMEMBER, $1, $2, 
									ADD_NODE2(T_FUNC, $4, $6)); }
	| T_TRY '{' statement_list '}' T_CATCH '{' statement_list '}' T_FINALLY '{' statement_list '}'
	            { $$ = ADD_NODE3(T_TRY, $3, $7, $11); }
	| T_TRY '{' statement_list '}' T_CATCH '{' statement_list '}'
	            { $$ = ADD_NODE2(T_CATCH, $3, $7); }
	| T_TRY '{' statement_list '}' T_FINALLY '{' statement_list '}'
	            { $$ = ADD_NODE2(T_FINALLY, $3, $7); }
	| T_TRY T_SEPARATOR statement_list T_CATCH T_SEPARATOR statement_list T_FINALLY T_SEPARATOR statement_list T_END T_SEPARATOR
	            { $$ = ADD_NODE3(T_TRY, $3, $6, $9); }
	| T_TRY T_SEPARATOR statement_list T_CATCH T_SEPARATOR statement_list T_END T_SEPARATOR
	            { $$ = ADD_NODE2(T_CATCH, $3, $6); }
	| T_TRY T_SEPARATOR statement_list T_FINALLY T_SEPARATOR statement_list T_END T_SEPARATOR
	            { $$ = ADD_NODE2(T_FINALLY, $3, $6); }
	;

lvalue_set:
	bareword				{ $$ = ADD_NODE1('$', $1); }
	| expression T_ARROW expression
							{ $$ = ADD_NODE2(T_ARROW, $1, $3); }
	| expression '.' bareword
							{ $$ = ADD_NODE2('.', $1, $3); }
/*	| lvalue_list bareword	{ $$ = ADD_NODE2(',', $1, $2); } */
	;

/* lvalue_list:
//	lvalue_list bareword ','
//							{ $$ = ADD_NODE2(',', $1, $2); }
//	|						{ $$ = ADD_NODE0(','); } */

expression:
	T_INTEGER				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_NULL				{ $$ = ADD_NODE0(T_LITERAL); }
	| T_BOOL				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_FLOAT				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| bareword				{ $$ = ADD_NODE1('$', $1); }
	| string				{ $$ = $1; }
	| '(' expression ')'	{ $$ = $2; }
	| '~' expression		{ $$ = ADD_NODE1('~', $2); }
	| '!' expression		{ $$ = ADD_NODE1('!', $2); }
	| '-' expression %prec T_NEGATIVE
							{ $$ = ADD_NODE1(T_NEGATIVE, $2); }
	| expression T_ARROW expression
							{ $$ = ADD_NODE2(T_ARROW, $1, $3); }
	| expression '.' bareword
							{ $$ = ADD_NODE2('.', $1, $3); }
	| '@' T_TYPE expression %prec '@'
							{ $$ = ADD_NODE2('@', $2, $3); }
	| expression ',' expression
							{ $$ = ADD_NODE2(',', $1, $3); }
	| expression T_EQ expression
							{ $$ = ADD_NODE2(T_EQ, $1, $3); }
	| expression '>' expression
							{ $$ = ADD_NODE2('>', $1, $3); }
	| expression '<' expression
							{ $$ = ADD_NODE2('<', $1, $3); }
	| expression T_SE expression
							{ $$ = ADD_NODE2(T_SE, $1, $3); }
	| expression T_GE expression
							{ $$ = ADD_NODE2(T_GE, $1, $3); }
	| expression T_LE expression
							{ $$ = ADD_NODE2(T_LE, $1, $3); }
	| expression T_NE expression
							{ $$ = ADD_NODE2(T_NE, $1, $3); }
	| expression T_SNE expression
							{ $$ = ADD_NODE2(T_SNE, $1, $3); }
	| expression T_COMPARE expression
	            			{ $$ = ADD_NODE2(T_COMPARE, $1, $3); }
	| expression '+' expression
							{ $$ = ADD_NODE2('+', $1, $3); }
	| expression '-' expression
							{ $$ = ADD_NODE2('-', $1, $3); }
	| expression '*' expression
							{ $$ = ADD_NODE2('*', $1, $3); }
	| expression '/' expression
							{ $$ = ADD_NODE2('/', $1, $3); }
	| expression '%' expression
							{ $$ = ADD_NODE2('%', $1, $3); }
	| expression '^' expression
							{ $$ = ADD_NODE2('^', $1, $3); }
	| expression '|' expression
							{ $$ = ADD_NODE2('|', $1, $3); }
	| expression '&' expression
							{ $$ = ADD_NODE2('&', $1, $3); }
	| expression T_AND expression
							{ $$ = ADD_NODE2(T_AND, $1, $3); }
	| expression T_OR expression
							{ $$ = ADD_NODE2(T_OR, $1, $3); }
	| expression T_XOR expression
							{ $$ = ADD_NODE2(T_XOR, $1, $3); }
	| expression T_RANGE expression
							{ $$ = ADD_NODE2(T_RANGE, $1, $3); }
	| expression ':' expression
							{ $$ = ADD_NODE2(':', $1, $3); }
	| '[' arraylist ']'		{ $$ = ADD_NODE1('[', $2); }
	| T_FUNC ':' parglist '{' statement_list '}'
							{ $$ = ADD_NODE2(T_FUNC, $3, $5); }
	| T_FUNC ':' parglist T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE2(T_FUNC, $3, $5); }
	| T_FUNC ':' parglist T_SEPARATOR statement_list T_ENDFUNC
							{ $$ = ADD_NODE2(T_FUNC, $3, $5); }
	| shortfunc expression
							{ $$ = ADD_NODE2(T_FUNC, $1, $2); }
	| T_CLASS T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_CLASS T_SEPARATOR statement_list T_ENDCLASS
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_CLASS '{' statement_list '}'
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_GIVEN expression '{' exprcaselist '}'
							{ $$ = ADD_NODE2(T_GIVEN, $2, $4); }
	| T_GIVEN expression T_SEPARATOR exprcaselist T_END
							{ $$ = ADD_NODE2(T_GIVEN, $2, $4); }
	| T_DOLLARPAREN command ')'
							{ $$ = $2; }
	| '{' anonclasslist '}'	{ $$ = ADD_NODE1('{', $2); }
	;

simple_expr:
	T_INTEGER				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_NULL				{ $$ = ADD_NODE0(T_LITERAL); }
	| T_BOOL				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_FLOAT				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| bareword				{ $$ = ADD_NODE1('$', $1); }
	| string				{ $$ = $1; }
	| '(' expression ')'	{ $$ = $2; }
	| '~' simple_expr		{ $$ = ADD_NODE1('~', $2); }
	| '!' simple_expr		{ $$ = ADD_NODE1('!', $2); }
	| simple_expr T_ARROW simple_expr
							{ $$ = ADD_NODE2(T_ARROW, $1, $3); }
	| simple_expr '.' bareword
							{ $$ = ADD_NODE2('.', $1, $3); }
	| '@' T_TYPE expression %prec '@'	
							{ $$ = ADD_NODE2('@', $2, $3); }
	| simple_expr T_EQ simple_expr
							{ $$ = ADD_NODE2(T_EQ, $1, $3); }
	| simple_expr '<' simple_expr
							{ $$ = ADD_NODE2('<', $1, $3); }
	| simple_expr T_SE simple_expr
							{ $$ = ADD_NODE2(T_SE, $1, $3); }
	| simple_expr T_GE simple_expr
							{ $$ = ADD_NODE2(T_GE, $1, $3); }
	| simple_expr T_LE simple_expr
							{ $$ = ADD_NODE2(T_LE, $1, $3); }
	| simple_expr T_NE simple_expr
							{ $$ = ADD_NODE2(T_NE, $1, $3); }
	| simple_expr T_SNE simple_expr
							{ $$ = ADD_NODE2(T_SNE, $1, $3); }
	| simple_expr T_COMPARE simple_expr
	            			{ $$ = ADD_NODE2(T_COMPARE, $1, $3); }
	| simple_expr '+' simple_expr
							{ $$ = ADD_NODE2('+', $1, $3); }
	| simple_expr '*' simple_expr
							{ $$ = ADD_NODE2('*', $1, $3); }
	| simple_expr '/' simple_expr
							{ $$ = ADD_NODE2('/', $1, $3); }
	| simple_expr '%' simple_expr
							{ $$ = ADD_NODE2('%', $1, $3); }
	| simple_expr '^' simple_expr
							{ $$ = ADD_NODE2('^', $1, $3); }
	| simple_expr '|' simple_expr
							{ $$ = ADD_NODE2('|', $1, $3); }
	| simple_expr '&' simple_expr
							{ $$ = ADD_NODE2('&', $1, $3); }
	| simple_expr T_AND simple_expr
							{ $$ = ADD_NODE2(T_AND, $1, $3); }
	| simple_expr T_OR simple_expr
							{ $$ = ADD_NODE2(T_OR, $1, $3); }
	| simple_expr T_XOR simple_expr
							{ $$ = ADD_NODE2(T_XOR, $1, $3); }
	| simple_expr T_RANGE simple_expr
							{ $$ = ADD_NODE2(T_RANGE, $1, $3); }
	| simple_expr ':' simple_expr
							{ $$ = ADD_NODE2(':', $1, $3); }
	| '[' arraylist ']'		{ $$ = ADD_NODE1('[', $2); }
	| T_CLASS T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_CLASS T_SEPARATOR statement_list T_ENDCLASS
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_CLASS '{' statement_list '}'
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_GIVEN expression '{' exprcaselist '}'
							{ $$ = ADD_NODE2(T_GIVEN, $2, $4); }
	| T_GIVEN expression T_SEPARATOR exprcaselist T_END
							{ $$ = ADD_NODE2(T_GIVEN, $2, $4); }
	| T_DOLLARPAREN command ')'
							{ $$ = $2; }
	| '{' anonclasslist '}'	{ $$ = ADD_NODE1('{', $2); }
	;

shortfunc:
	T_FUNC ':' parglist T_ARROW
							{ $$ = $3; }
	;

command:
	'$' bareword paralist		{ $$ = ADD_NODE2(T_COMMAND, $2, $3); }
	;

paralist:
	para paralist			{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

para:
	simple_expr					{ $$ = $1; }
	| T_MINMIN bareword_or_string '=' expression
							{ $$ = ADD_NODE2(T_LONGPARA, $2, $4); }
	| T_MINMIN bareword_or_string
							{ $$ = ADD_NODE1(T_LONGPARA, $2); }
	| '-' bareword_or_string
							{ $$ = ADD_NODE1(T_SHORTPARA, $2); }
	| '-' bareword_or_string '=' expression
							{ $$ = ADD_NODE2(T_SHORTPARA, $2, $4); }
	| '>' bareword_or_string
							{ $$ = ADD_NODE1(T_REDIRECT, $2); }
	| '}' bareword_or_string
							{ $$ = ADD_NODE1('}', $2); }
	;
	
bareword_or_string:
	bareword				{ $$ = $1; }
	| string				{ $$ = $1; }

bareword:
	T_VARIABLE				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	;

string:
	T_STRING				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	;

arraylist:
	arraylist_i arraymember ','
							{ $$ = ADD_NODE2(',', $1, $2); }
	| arraylist_i arraymember
							{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

arraylist_i:
	arraylist_i arraymember ','
							{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

arraymember:
	simple_expr T_DOUBLEARROW simple_expr
							{ $$ = ADD_NODE2(T_ARRAYMEMBER, $1, $3); }
	| simple_expr			{ $$ = ADD_NODE1(T_ARRAYMEMBER, $1); }
	;

anonclasslist:
	anonclasslist_i anonclassmember ','
							{ $$ = ADD_NODE2(',', $1, $2); }
	| anonclasslist_i anonclassmember
							{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

anonclasslist_i:
	anonclasslist_i anonclassmember ','
							{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

anonclassmember:
	bareword_or_string ':' simple_expr
							{ $$ = ADD_NODE2(T_ARRAYMEMBER, $1, $3); }
	;

parglist:
	parglist parg			{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

parg:
	bareword				{ $$ = $1; }
	| bareword ','			{ $$ = $1; }
	;

caselist:
	acase caselist			{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

acase:
	T_CASE expression T_SEPARATOR statement_list
							{ $$ = ADD_NODE2(T_CASE, $2, $4); }
	| T_DEFAULT T_SEPARATOR statement_list
							{ $$ = ADD_NODE1(T_CASE, $3); }
	;

exprcaselist:
	exprcase exprcaselist	{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

exprcase:
	T_CASE expression T_SEPARATOR expression T_SEPARATOR
							{ $$ = ADD_NODE2(T_CASE, $2, $4); }
	| T_DEFAULT T_SEPARATOR expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_CASE, $3); }
	;

attributelist:
	attributelist_inner T_ATTRIBUTE
							{ $$ = ADD_NODE2(T_ATTRIBUTE, $1, $2); }

attributelist_inner:
	attributelist_inner T_ATTRIBUTE
							{ $$ = ADD_NODE2(T_ATTRIBUTE, $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(T_ATTRIBUTE); }
	;

%%
int eh_outer_exit(int exitval) {
	//free_node: something. We should actually be adding stuff to the AST, I suppose.
	return exitval;
}
