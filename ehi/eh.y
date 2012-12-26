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
EHI *yyget_extra(void *scanner);
#define YYERROR_VERBOSE
#define YYLEX_PARAM scanner

// can't overload macros
#define ADD_NODE0(opcode) eh_addnode(opcode)
#define ADD_NODE1(opcode, first) eh_addnode(opcode, ehretval_t::make(first))
#define ADD_NODE2(opcode, first, second) eh_addnode(opcode, ehretval_t::make(first), ehretval_t::make(second))
#define ADD_NODE3(opcode, first, second, third) eh_addnode(opcode, ehretval_t::make(first), ehretval_t::make(second), ehretval_t::make(third))
#define ADD_NODE4(opcode, first, second, third, fourth) eh_addnode(opcode, ehretval_t::make(first), ehretval_t::make(second), ehretval_t::make(third), ehretval_t::make(fourth))
#define ADD_COMPOUND(opcode, lval, rval, result) ehretval_p lvalue = ehretval_t::make(lval); result = eh_addnode('=', lvalue, ehretval_t::make(eh_addnode(opcode, lvalue, ehretval_t::make(rval))))

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
%token <bValue> T_BOOL
%token T_IF
%token T_ELSE
%token T_ELSIF
%token T_WHILE
%token T_FOR
%token T_AS
%token T_IN
%token T_GIVEN
%token T_MATCH
%token T_END
%token T_SWITCH
%token T_DEFAULT
%token T_CASE
%token T_BREAK
%token T_CONTINUE
%token T_FUNC
%token T_RET
%token T_SEPARATOR
%token T_NULL
%token T_ENUM
%token T_CLASS
%token T_CLASSMEMBER
%token T_LITERAL
%token T_TRY
%token T_CATCH
%token T_FINALLY
%token <vValue> T_ATTRIBUTE
%token T_ARRAYMEMBER
%token T_DOUBLEARROW
%token T_COMMAND T_SHORTPARA T_LONGPARA T_REDIRECT
%token <sValue> T_VARIABLE
%token <sValue> T_STRING
%token <sValue> T_CUSTOMOP
%right '=' T_PLUSEQ T_MINEQ T_MULTIPLYEQ T_DIVIDEEQ T_MODULOEQ T_ANDEQ T_OREQ T_XOREQ T_BINANDEQ T_BINOREQ T_BINXOREQ T_LEFTSHIFTEQ T_RIGHTSHIFTEQ
%right T_DOUBLEARROW
%right ','
%left T_AND T_OR T_XOR
%left '|' '^' '&'
%right T_CUSTOMOP
%left '>' '<' T_GE T_LE T_NE T_SE T_SNE T_EQ T_COMPARE
%left T_LEFTSHIFT T_RIGHTSHIFT
%left '+' '-'
%left '*' '/' '%'
%nonassoc T_PLUSPLUS T_MINMIN
%right '@'
%nonassoc '~' '!' T_NEGATIVE
%left T_RANGE T_ARROW '.'
%right ':'
%nonassoc '[' ']' '{' '}'
%nonassoc '(' ')' T_DOLLARPAREN
%nonassoc T_INTEGER T_FLOAT T_NULL T_BOOL T_VARIABLE T_STRING T_GIVEN T_MATCH T_FUNC T_CLASS T_IF T_THIS T_SCOPE '_'

%type<ehNode> statement expression statement_list parglist arraylist arraymember arraylist_i anonclasslist anonclassmember
%type<ehNode> anonclasslist_i attributelist attributelist_inner caselist acase command paralist para global_list
%type<ehNode> bareword_or_string para_expr catch_clauses catch_clause block_expression
%type<ehNode> elseif_clauses elseif_clause enum_list enum_member enum_arg_list
%%
program:
	global_list				{ 	// Don't do anything. Destructors below take
								// care of cleanup.
							}

global_list:
	/* NULL */				{ }
	| statement				{
								EHI *ehi = yyget_extra(scanner);
								ehretval_p statement = ehretval_t::make($1);
								ehretval_p ret = ehi->eh_execute(statement, ehi->get_context());
								if(ehi->get_interactivity() != end_is_end_e) {
									// TODO: make this use printvar instead
									std::cout << "=> " << ehi->to_string(ret, ehi->get_context())->get_stringval() << std::endl;
								}
#if defined(DEBUG_GC) || defined(RUN_GC)
								EHInterpreter *interpreter = ehi->get_parent();
								interpreter->gc.do_collect(interpreter->global_object);
#endif
								// flush stdout after executing each statement
								//fflush(stdout);
								if(ehi->get_returning()) {
									return (ret->type() == int_e) ? ret->get_intval() : 0;
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
		/* Using T_END */
	| T_WHILE block_expression T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_WHILE, $2, $4); }
	| T_FOR block_expression T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_IN, $2, $4); }
	| T_FOR block_expression T_IN block_expression T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE3(T_IN, $2, $4, $6); }
	| T_FUNC T_VARIABLE ':' parglist T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE2('=',
								ADD_NODE1('$', $2),
								ADD_NODE2(T_FUNC, $4, $6)); }
	| T_SWITCH block_expression T_SEPARATOR caselist T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_SWITCH, $2, $4); }
	| T_ENUM T_VARIABLE T_SEPARATOR enum_list T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE3(T_ENUM, $2, $4, $6); }
	| T_CLASS T_VARIABLE T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE2(T_CLASS, $2, $4); }
		/* Other statements */
	| T_RET expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_RET, $2); }
	| T_RET T_SEPARATOR		{ $$ = ADD_NODE0(T_RET); }
	| T_CONTINUE T_SEPARATOR
							{ $$ = ADD_NODE0(T_CONTINUE); }
	| T_CONTINUE expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_CONTINUE, $2); }
	| T_BREAK T_SEPARATOR	{ $$ = ADD_NODE0(T_BREAK); }
	| T_BREAK expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_BREAK, $2); }
		/* property declaration */
	| attributelist T_VARIABLE T_SEPARATOR
							{ $$ = ADD_NODE3(T_CLASSMEMBER, $1, ADD_NODE1('$', $2), ADD_NODE0(T_NULL)); }
	| attributelist expression '=' expression T_SEPARATOR
							{ $$ = ADD_NODE3(T_CLASSMEMBER, $1, $2, $4); }
	| attributelist T_VARIABLE ':' parglist T_SEPARATOR statement_list T_END T_SEPARATOR
							{ $$ = ADD_NODE3(T_CLASSMEMBER, $1, ADD_NODE1('$', $2),
									ADD_NODE2(T_FUNC, $4, $6)); }
	| T_TRY T_SEPARATOR statement_list catch_clauses T_END T_SEPARATOR
				{ $$ = ADD_NODE2(T_TRY, $3, $4); }
	| T_TRY T_SEPARATOR statement_list catch_clauses T_FINALLY T_SEPARATOR statement_list T_END T_SEPARATOR
				{ $$ = ADD_NODE3(T_TRY, $3, $4, $7); }
	| '$' command T_SEPARATOR	{ $$ = $2; }
	;

catch_clauses:
	catch_clause catch_clauses
							{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

catch_clause:
	T_CATCH T_SEPARATOR statement_list
							{ $$ = ADD_NODE1(T_CATCH, $3); }
	| T_CATCH T_IF expression T_SEPARATOR statement_list
							{ $$ = ADD_NODE2(T_CATCH, $3, $5); }
	;

expression:
	T_INTEGER				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_NULL				{ $$ = ADD_NODE0(T_NULL); }
	| T_BOOL				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_FLOAT				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_VARIABLE			{ $$ = ADD_NODE1('$', $1); }
	| T_STRING				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_THIS				{ $$ = ADD_NODE0(T_THIS); }
	| T_SCOPE				{ $$ = ADD_NODE0(T_SCOPE); }
	| '_'					{ $$ = ADD_NODE0('_'); }
	| '(' expression ')'	{ $$ = ADD_NODE1('(', $2); }
	| '~' expression		{ $$ = ADD_NODE1('~', $2); }
	| '!' expression		{ $$ = ADD_NODE1('!', $2); }
	| expression T_PLUSPLUS	{
								ehretval_p lvalue = ehretval_t::make($1);
								$$ = eh_addnode('=', lvalue, ehretval_t::make(eh_addnode('+', lvalue, ehretval_t::make(1))));
							}
	| expression T_MINMIN	{
								ehretval_p lvalue = ehretval_t::make($1);
								$$ = eh_addnode('=', lvalue, ehretval_t::make(eh_addnode('-', lvalue, ehretval_t::make(1))));
							}
	| expression T_CUSTOMOP expression
							{ $$ = ADD_NODE3(T_CUSTOMOP, $1, $2, $3); }
	| expression '=' expression
							{ $$ = ADD_NODE2('=', $1, $3); }
	| expression T_PLUSEQ expression
							{ ADD_COMPOUND('+', $1, $3, $$); }
	| expression T_MINEQ expression
							{ ADD_COMPOUND('-', $1, $3, $$); }
	| expression T_MULTIPLYEQ expression
							{ ADD_COMPOUND('*', $1, $3, $$); }
	| expression T_DIVIDEEQ expression
							{ ADD_COMPOUND('/', $1, $3, $$); }
	| expression T_MODULOEQ expression
							{ ADD_COMPOUND('%', $1, $3, $$); }
	| expression T_ANDEQ expression
							{ ADD_COMPOUND(T_AND, $1, $3, $$); }
	| expression T_OREQ expression
							{ ADD_COMPOUND(T_OR, $1, $3, $$); }
	| expression T_XOREQ expression
							{ ADD_COMPOUND(T_XOR, $1, $3, $$); }
	| expression T_BINANDEQ expression
							{ ADD_COMPOUND('&', $1, $3, $$); }
	| expression T_BINOREQ expression
							{ ADD_COMPOUND('|', $1, $3, $$); }
	| expression T_BINXOREQ expression
							{ ADD_COMPOUND('^', $1, $3, $$); }
	| expression T_LEFTSHIFTEQ expression
							{ ADD_COMPOUND(T_LEFTSHIFT, $1, $3, $$); }
	| expression T_RIGHTSHIFTEQ expression
							{ ADD_COMPOUND(T_RIGHTSHIFT, $1, $3, $$); }
	| expression T_ARROW expression
							{ $$ = ADD_NODE2(T_ARROW, $1, $3); }
	| expression T_DOUBLEARROW expression
							{ $$ = ADD_NODE2(T_FUNC, $1, $3); }
	| expression '.' T_VARIABLE
							{ $$ = ADD_NODE2('.', $1, $3); }
	| '@' T_VARIABLE		{ $$ = ADD_NODE1('@', $2); }
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
	| expression T_LEFTSHIFT expression
							{ $$ = ADD_NODE2(T_LEFTSHIFT, $1, $3); }
	| expression T_RIGHTSHIFT expression
							{ $$ = ADD_NODE2(T_RIGHTSHIFT, $1, $3); }
	| expression %prec ':' expression
							{ $$ = ADD_NODE2(':', $1, $2); }
	| '[' arraylist ']'		{ $$ = ADD_NODE1('[', $2); }
	| T_FUNC ':' parglist T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE2(T_FUNC, $3, $5); }
	| T_CLASS T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_GIVEN block_expression T_SEPARATOR caselist T_END
							{ $$ = ADD_NODE2(T_GIVEN, $2, $4); }
	| T_MATCH block_expression T_SEPARATOR caselist T_END
							{ $$ = ADD_NODE2(T_MATCH, $2, $4); }
	| '(' '$' command ')'
							{ $$ = $3; }
	| '{' anonclasslist '}'	{ $$ = ADD_NODE1('{', $2); }

	| T_IF block_expression T_SEPARATOR statement_list elseif_clauses T_END
							{ $$ = ADD_NODE3(T_IF, $2, $4, $5); }
	| T_IF block_expression T_SEPARATOR statement_list elseif_clauses T_ELSE statement_list T_END
							{ $$ = ADD_NODE4(T_IF, $2, $4, $5, $7); }
	;

elseif_clauses:
	elseif_clause elseif_clauses
							{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

elseif_clause:
	T_ELSIF block_expression T_SEPARATOR statement_list
							{ $$ = ADD_NODE2(T_ELSE, $2, $4); }
	;


block_expression:
	/*
	 * Expression as used in block headers, e.g.
	 * if block_expression {
	 *		statement_list
	 * }
	 *
	 * Because of ambiguities with braces, this does not allow hash literals; otherwise identical to expression.
	 */
	T_INTEGER				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_NULL				{ $$ = ADD_NODE0(T_NULL); }
	| T_BOOL				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_FLOAT				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_VARIABLE			{ $$ = ADD_NODE1('$', $1); }
	| T_STRING				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_THIS				{ $$ = ADD_NODE0(T_THIS); }
	| T_SCOPE				{ $$ = ADD_NODE0(T_SCOPE); }
	| '(' expression ')'	{ $$ = ADD_NODE1('(', $2); }
	| '~' block_expression	{ $$ = ADD_NODE1('~', $2); }
	| '!' block_expression		{ $$ = ADD_NODE1('!', $2); }
	| block_expression T_PLUSPLUS	{
								ehretval_p lvalue = ehretval_t::make($1);
								$$ = eh_addnode('=', lvalue, ehretval_t::make(eh_addnode('+', lvalue, ehretval_t::make(1))));
							}
	| block_expression T_MINMIN	{
								ehretval_p lvalue = ehretval_t::make($1);
								$$ = eh_addnode('=', lvalue, ehretval_t::make(eh_addnode('-', lvalue, ehretval_t::make(1))));
							}
	| block_expression T_CUSTOMOP block_expression
							{ $$ = ADD_NODE3(T_CUSTOMOP, $1, $2, $3); }
	| block_expression '=' block_expression
							{ $$ = ADD_NODE2('=', $1, $3); }
	| block_expression T_PLUSEQ block_expression
							{ ADD_COMPOUND('+', $1, $3, $$); }
	| block_expression T_MINEQ block_expression
							{ ADD_COMPOUND('-', $1, $3, $$); }
	| block_expression T_MULTIPLYEQ block_expression
							{ ADD_COMPOUND('*', $1, $3, $$); }
	| block_expression T_DIVIDEEQ block_expression
							{ ADD_COMPOUND('/', $1, $3, $$); }
	| block_expression T_MODULOEQ block_expression
							{ ADD_COMPOUND('%', $1, $3, $$); }
	| block_expression T_ANDEQ block_expression
							{ ADD_COMPOUND(T_AND, $1, $3, $$); }
	| block_expression T_OREQ block_expression
							{ ADD_COMPOUND(T_OR, $1, $3, $$); }
	| block_expression T_XOREQ block_expression
							{ ADD_COMPOUND(T_XOR, $1, $3, $$); }
	| block_expression T_BINANDEQ block_expression
							{ ADD_COMPOUND('&', $1, $3, $$); }
	| block_expression T_BINOREQ block_expression
							{ ADD_COMPOUND('|', $1, $3, $$); }
	| block_expression T_BINXOREQ block_expression
							{ ADD_COMPOUND('^', $1, $3, $$); }
	| block_expression T_LEFTSHIFTEQ block_expression
							{ ADD_COMPOUND(T_LEFTSHIFT, $1, $3, $$); }
	| block_expression T_RIGHTSHIFTEQ block_expression
							{ ADD_COMPOUND(T_RIGHTSHIFT, $1, $3, $$); }
	| block_expression T_ARROW block_expression
							{ $$ = ADD_NODE2(T_ARROW, $1, $3); }
	| block_expression T_DOUBLEARROW block_expression
							{ $$ = ADD_NODE2(T_FUNC, $1, $3); }
	| block_expression '.' T_VARIABLE
							{ $$ = ADD_NODE2('.', $1, $3); }
	| block_expression ',' block_expression
							{ $$ = ADD_NODE2(',', $1, $3); }
	| block_expression T_EQ block_expression
							{ $$ = ADD_NODE2(T_EQ, $1, $3); }
	| block_expression '>' block_expression
							{ $$ = ADD_NODE2('>', $1, $3); }
	| block_expression '<' block_expression
							{ $$ = ADD_NODE2('<', $1, $3); }
	| block_expression T_SE block_expression
							{ $$ = ADD_NODE2(T_SE, $1, $3); }
	| block_expression T_GE block_expression
							{ $$ = ADD_NODE2(T_GE, $1, $3); }
	| block_expression T_LE block_expression
							{ $$ = ADD_NODE2(T_LE, $1, $3); }
	| block_expression T_NE block_expression
							{ $$ = ADD_NODE2(T_NE, $1, $3); }
	| block_expression T_SNE block_expression
							{ $$ = ADD_NODE2(T_SNE, $1, $3); }
	| block_expression T_COMPARE block_expression
	            			{ $$ = ADD_NODE2(T_COMPARE, $1, $3); }
	| block_expression '+' block_expression
							{ $$ = ADD_NODE2('+', $1, $3); }
	| block_expression '-' block_expression
							{ $$ = ADD_NODE2('-', $1, $3); }
	| block_expression '*' block_expression
							{ $$ = ADD_NODE2('*', $1, $3); }
	| block_expression '/' block_expression
							{ $$ = ADD_NODE2('/', $1, $3); }
	| block_expression '%' block_expression
							{ $$ = ADD_NODE2('%', $1, $3); }
	| block_expression '^' block_expression
							{ $$ = ADD_NODE2('^', $1, $3); }
	| block_expression '|' block_expression
							{ $$ = ADD_NODE2('|', $1, $3); }
	| block_expression '&' block_expression
							{ $$ = ADD_NODE2('&', $1, $3); }
	| block_expression T_AND block_expression
							{ $$ = ADD_NODE2(T_AND, $1, $3); }
	| block_expression T_OR block_expression
							{ $$ = ADD_NODE2(T_OR, $1, $3); }
	| block_expression T_XOR block_expression
							{ $$ = ADD_NODE2(T_XOR, $1, $3); }
	| block_expression T_RANGE block_expression
							{ $$ = ADD_NODE2(T_RANGE, $1, $3); }
	| block_expression T_LEFTSHIFT block_expression
							{ $$ = ADD_NODE2(T_LEFTSHIFT, $1, $3); }
	| block_expression T_RIGHTSHIFT block_expression
							{ $$ = ADD_NODE2(T_RIGHTSHIFT, $1, $3); }
	| block_expression %prec ':' block_expression
							{ $$ = ADD_NODE2(':', $1, $2); }
	| '[' arraylist ']'		{ $$ = ADD_NODE1('[', $2); }
	| T_FUNC ':' parglist T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE2(T_FUNC, $3, $5); }
	| T_CLASS T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_GIVEN block_expression T_SEPARATOR caselist T_END
							{ $$ = ADD_NODE2(T_GIVEN, $2, $4); }
	| T_MATCH block_expression T_SEPARATOR caselist T_END
							{ $$ = ADD_NODE2(T_MATCH, $2, $4); }
	| '(' '$' command ')'
							{ $$ = $3; }
	| T_IF block_expression T_SEPARATOR statement_list elseif_clauses T_END
							{ $$ = ADD_NODE3(T_IF, $2, $4, $5); }
	| T_IF block_expression T_SEPARATOR statement_list elseif_clauses T_ELSE statement_list T_END
							{ $$ = ADD_NODE4(T_IF, $2, $4, $5, $7); }
	;

para_expr:
	/*
	 * Expression used in command arguments and array and hash literal members. Disallows tuples and function calls.
	 */
	T_INTEGER				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_NULL				{ $$ = ADD_NODE0(T_NULL); }
	| T_BOOL				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_FLOAT				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_VARIABLE			{ $$ = ADD_NODE1('$', $1); }
	| T_STRING				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_THIS				{ $$ = ADD_NODE0(T_THIS); }
	| T_SCOPE				{ $$ = ADD_NODE0(T_SCOPE); }
	| '(' expression ')'	{ $$ = ADD_NODE1('(', $2); }
	| '~' para_expr			{ $$ = ADD_NODE1('~', $2); }
	| '!' para_expr			{ $$ = ADD_NODE1('!', $2); }
	| para_expr T_ARROW para_expr
							{ $$ = ADD_NODE2(T_ARROW, $1, $3); }
	| para_expr '.' T_VARIABLE
							{ $$ = ADD_NODE2('.', $1, $3); }
	| para_expr T_EQ para_expr
							{ $$ = ADD_NODE2(T_EQ, $1, $3); }
	| para_expr '<' para_expr
							{ $$ = ADD_NODE2('<', $1, $3); }
	| para_expr T_SE para_expr
							{ $$ = ADD_NODE2(T_SE, $1, $3); }
	| para_expr T_GE para_expr
							{ $$ = ADD_NODE2(T_GE, $1, $3); }
	| para_expr T_LE para_expr
							{ $$ = ADD_NODE2(T_LE, $1, $3); }
	| para_expr T_NE para_expr
							{ $$ = ADD_NODE2(T_NE, $1, $3); }
	| para_expr T_SNE para_expr
							{ $$ = ADD_NODE2(T_SNE, $1, $3); }
	| para_expr T_COMPARE para_expr
	            			{ $$ = ADD_NODE2(T_COMPARE, $1, $3); }
	| para_expr '+' para_expr
							{ $$ = ADD_NODE2('+', $1, $3); }
	| para_expr '*' para_expr
							{ $$ = ADD_NODE2('*', $1, $3); }
	| para_expr '/' para_expr
							{ $$ = ADD_NODE2('/', $1, $3); }
	| para_expr '%' para_expr
							{ $$ = ADD_NODE2('%', $1, $3); }
	| para_expr '^' para_expr
							{ $$ = ADD_NODE2('^', $1, $3); }
	| para_expr '|' para_expr
							{ $$ = ADD_NODE2('|', $1, $3); }
	| para_expr '&' para_expr
							{ $$ = ADD_NODE2('&', $1, $3); }
	| para_expr T_AND para_expr
							{ $$ = ADD_NODE2(T_AND, $1, $3); }
	| para_expr T_OR para_expr
							{ $$ = ADD_NODE2(T_OR, $1, $3); }
	| para_expr T_XOR para_expr
							{ $$ = ADD_NODE2(T_XOR, $1, $3); }
	| para_expr T_RANGE para_expr
							{ $$ = ADD_NODE2(T_RANGE, $1, $3); }
	| '[' arraylist ']'		{ $$ = ADD_NODE1('[', $2); }
	| T_CLASS T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_GIVEN para_expr T_SEPARATOR caselist T_END
							{ $$ = ADD_NODE2(T_GIVEN, $2, $4); }
	| T_MATCH block_expression T_SEPARATOR caselist T_END
							{ $$ = ADD_NODE2(T_MATCH, $2, $4); }
	| '(' '$' command ')'
							{ $$ = $3; }
	| '{' anonclasslist '}'	{ $$ = ADD_NODE1('{', $2); }
	;

command:
	T_VARIABLE paralist		{ $$ = ADD_NODE2(T_COMMAND, $1, $2); }
	;

paralist:
	para paralist			{ $$ = ADD_NODE2(',', $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(','); }
	;

para:
	para_expr					{ $$ = $1; }
	| T_MINMIN bareword_or_string '=' para_expr
							{ $$ = ADD_NODE2(T_LONGPARA, $2, $4); }
	| T_MINMIN bareword_or_string
							{ $$ = ADD_NODE1(T_LONGPARA, $2); }
	| '-' bareword_or_string
							{ $$ = ADD_NODE1(T_SHORTPARA, $2); }
	| '-' bareword_or_string '=' para_expr
							{ $$ = ADD_NODE2(T_SHORTPARA, $2, $4); }
	| '>' bareword_or_string
							{ $$ = ADD_NODE1(T_REDIRECT, $2); }
	| '}' bareword_or_string
							{ $$ = ADD_NODE1('}', $2); }
	;

bareword_or_string:
	T_VARIABLE				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_STRING				{ $$ = ADD_NODE1(T_LITERAL, $1); }

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
	para_expr T_DOUBLEARROW para_expr
							{ $$ = ADD_NODE2(T_ARRAYMEMBER, $1, $3); }
	| para_expr				{ $$ = ADD_NODE1(T_ARRAYMEMBER, $1); }
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
	T_VARIABLE ':' para_expr
							{ $$ = ADD_NODE2(T_ARRAYMEMBER, $1, $3); }
	| T_STRING ':' para_expr
							{ $$ = ADD_NODE2(T_ARRAYMEMBER, $1, $3); }
	;

parglist:
	block_expression		{ $$ = $1; }
	| /* NULL */			{ $$ = ADD_NODE0(T_NULL); }
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

attributelist:
	attributelist_inner T_ATTRIBUTE
							{ $$ = ADD_NODE2(T_ATTRIBUTE, $1, $2); }

attributelist_inner:
	attributelist_inner T_ATTRIBUTE
							{ $$ = ADD_NODE2(T_ATTRIBUTE, $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(T_ATTRIBUTE); }
	;

enum_list:
	enum_member				{ $$ = $1; }
	| enum_list ',' enum_member
							{ $$ = ADD_NODE2(',', $1, $3); }
	;

enum_member:
	T_VARIABLE				{ $$ = ADD_NODE1(T_ENUM, $1); }
	| T_VARIABLE '(' enum_arg_list ')'
							{ $$ = ADD_NODE2(T_ENUM, $1, $3); }
	;

enum_arg_list:
	T_VARIABLE				{ $$ = ADD_NODE1(T_LITERAL, $1); }
	| T_VARIABLE ',' enum_arg_list
							{ $$ = ADD_NODE2(',', $1, $3); }
	;
%%
int eh_outer_exit(int exitval) {
	//free_node: something. We should actually be adding stuff to the AST, I suppose.
	return exitval;
}
