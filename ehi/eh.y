%{
/*
 * eh.y
 * Jelle Zijlstra, December 2011
 *
 * Yacc grammar specification for the EH scripting language. The first versions
 * of this code were inspired by Tom Niemann's "A Compact Guide to Lex & Yacc",
 * available at http://epaperpress.com/lexandyacc/
 */
#include "eh.hpp"
#include "std_lib/Integer.hpp"
#include "std_lib/String.hpp"
#include "std_lib/Float.hpp"
#include "std_lib/Bool.hpp"
#include "std_lib/Node.hpp"
#include "std_lib/Attribute.hpp"

#include "eh.bison.hpp"
#include "eh.flex.hpp"

#define YYERROR_VERBOSE
#define YYLEX_PARAM scanner

// can't overload macros
#define PARENT yyget_extra(scanner)->get_parent()
#define NODE(val) Node::make(val, PARENT)
#define ADD_NODE0(opcode) eh_addnode(opcode)
#define ADD_NODE1(opcode, first) eh_addnode(opcode, NODE(first))
#define ADD_NODE2(opcode, first, second) eh_addnode(opcode, NODE(first), NODE(second))
#define ADD_NODE3(opcode, first, second, third) eh_addnode(opcode, NODE(first), NODE(second), NODE(third))
#define ADD_NODE4(opcode, first, second, third, fourth) eh_addnode(opcode, NODE(first), NODE(second), NODE(third), NODE(fourth))
#define ADD_COMPOUND(opcode, lval, rval, result) ehval_p lvalue = NODE(lval); result = eh_addnode(T_ASSIGN, lvalue, NODE(eh_addnode(opcode, lvalue, NODE(rval))))

%}
%pure-parser
%parse-param { void *scanner }
%union {
	char *sValue;
	int iValue;
	float fValue;
	Attribute::attribute_enum vValue;
	bool bValue;
	Node *ehNode;
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
%token T_CLASS_MEMBER
%token T_LITERAL
%token T_TRY
%token T_CATCH
%token T_FINALLY
%token <vValue> T_ATTRIBUTE
%token T_ARRAY_MEMBER
%token T_DOUBLEARROW
%token T_CALL_METHOD T_TRY_FINALLY T_CATCH_IF T_FOR_IN T_NAMED_CLASS T_IF_ELSE T_NULLARY_ENUM T_ENUM_WITH_ARGUMENTS
%token T_ARRAY_MEMBER_NO_KEY T_ANYTHING T_GROUPING T_ASSIGN T_ADD T_SUBTRACT T_MULTIPLY T_DIVIDE T_MODULO T_GREATER
%token T_LESSER T_BINARY_AND T_BINARY_OR T_BINARY_XOR T_BINARY_COMPLEMENT T_NOT T_MATCH_SET T_COMMA T_ARRAY_LITERAL
%token T_HASH_LITERAL T_CALL T_ACCESS T_LIST T_NAMED_ARGUMENT T_MIXED_TUPLE
%token T_COMMAND T_SHORTPARA T_LONGPARA
%token <sValue> T_VARIABLE
%token <sValue> T_STRING
%token <sValue> T_CUSTOMOP
%nonassoc T_RAW
%right '=' T_PLUSEQ T_MINEQ T_MULTIPLYEQ T_DIVIDEEQ T_MODULOEQ T_ANDEQ T_OREQ T_XOREQ T_BINANDEQ T_BINOREQ T_BINXOREQ T_LEFTSHIFTEQ T_RIGHTSHIFTEQ
%right T_DOUBLEARROW
%nonassoc T_ATTRIBUTE
%right ','
%nonassoc ':'
%left T_AND T_OR T_XOR
%left '|' '^' '&'
%right T_CUSTOMOP
%left '>' '<' T_GE T_LE T_NE T_EQ T_COMPARE
%left T_LEFTSHIFT T_RIGHTSHIFT
%left '+' '-'
%left '*' '/' '%'
%nonassoc T_PLUSPLUS T_MINMIN
%right '@'
%nonassoc '~' '!' T_NEGATIVE
%left T_RANGE T_ARROW '.'
%right T_CALL
%nonassoc '[' ']' '{' '}'
%nonassoc '(' ')'
%nonassoc T_INTEGER T_FLOAT T_NULL T_BOOL T_VARIABLE T_STRING T_GIVEN T_MATCH T_SWITCH T_FUNC T_CLASS T_ENUM T_IF T_TRY T_FOR T_WHILE T_THIS T_SCOPE '_'

%type<ehNode> statement expression statement_list parglist arraylist arraymember arraylist_i anonclasslist anonclassmember
%type<ehNode> anonclasslist_i attributelist attributelist_inner caselist acase command paralist para global_list
%type<ehNode> para_expr catch_clauses catch_clause
%type<ehNode> elseif_clauses elseif_clause enum_list enum_member enum_arg_list
%type<sValue> bareword_or_string
%%
program:
	global_list				{
								EHI *ehi = yyget_extra(scanner);
								if(ehi->get_interactivity() == end_is_end_e) {
									ehval_p code = NODE($1);
									ehi->set_code(code);
								}
							}

global_list:
	/* NULL */				{ $$ = nullptr; }
	| statement				{
								EHI *ehi = yyget_extra(scanner);
								if(ehi->get_interactivity() != end_is_end_e) {
									ehval_p statement = NODE($1);
									ehval_p ret = ehi->eh_execute(statement, ehi->get_context());
									std::cout << "=> " << ehi->toString(ret, ehi->get_context())->get<String>() << std::endl;
#if defined(DEBUG_GC) || defined(RUN_GC)
									EHInterpreter *interpreter = ehi->get_parent();
									interpreter->gc.do_collect(interpreter->global_object);
#endif
									if(ehi->get_returning()) {
										return (ret->is_a<Integer>()) ? ret->get<Integer>() : 0;
									}
								}
							} global_list {
								EHI *ehi = yyget_extra(scanner);
								if(ehi->get_interactivity() == end_is_end_e) {
									$$ = ADD_NODE2(T_SEPARATOR, $1, $3);
								}
							}
	;

statement_list:
	/* NULL */				{ $$ = nullptr; }
	| statement statement_list
							{
								// get rid of empty statements
								if($1 == nullptr) {
									$$ = $2;
								} else if($2 == nullptr) {
									$$ = $1;
								} else {
									$$ = ADD_NODE2(T_SEPARATOR, $1, $2);
								}
							}
	;

statement:
	T_SEPARATOR				{ $$ = nullptr; }
	| expression T_SEPARATOR
							{ $$ = $1; }
	| '$' command T_SEPARATOR
							{ $$ = $2; }
		/* Other statements */
	| T_RET expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_RET, $2); }
	| T_RET T_SEPARATOR		{ $$ = eh_addnode(T_RET, Null::make()); }
	| T_CONTINUE T_SEPARATOR
							{ $$ = eh_addnode(T_CONTINUE, Integer::make(1)); }
	| T_CONTINUE expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_CONTINUE, $2); }
	| T_BREAK T_SEPARATOR	{ $$ = eh_addnode(T_BREAK, Integer::make(1)); }
	| T_BREAK expression T_SEPARATOR
							{ $$ = ADD_NODE1(T_BREAK, $2); }
		/* property declaration */
	;

expression:
	T_INTEGER				{ $$ = eh_addnode(T_LITERAL, Integer::make($1)); }
	| T_NULL				{ $$ = ADD_NODE0(T_NULL); }
	| T_BOOL				{ $$ = eh_addnode(T_LITERAL, Bool::make($1)); }
	| T_FLOAT				{ $$ = eh_addnode(T_LITERAL, Float::make($1)); }
	| T_VARIABLE			{ $$ = eh_addnode(T_VARIABLE, String::make($1)); }
	| T_STRING				{ $$ = eh_addnode(T_LITERAL, String::make($1)); }
	| T_THIS				{ $$ = ADD_NODE0(T_THIS); }
	| T_SCOPE				{ $$ = ADD_NODE0(T_SCOPE); }
	| '_'					{ $$ = ADD_NODE0(T_ANYTHING); }
	| '(' expression ')'	{ $$ = ADD_NODE1(T_GROUPING, $2); }
	| '~' expression		{ $$ = ADD_NODE1(T_BINARY_COMPLEMENT, $2); }
	| '!' expression		{ $$ = ADD_NODE1(T_NOT, $2); }
	| '@' T_VARIABLE		{ $$ = eh_addnode(T_MATCH_SET, String::make($2)); }
	| T_RAW expression		{ $$ = ADD_NODE1(T_RAW, $2); }
	| T_VARIABLE ':' expression %prec ':'
							{ $$ = eh_addnode(T_NAMED_ARGUMENT, String::make($1), NODE($3)); }
	| expression T_PLUSPLUS	{
								ehval_p lvalue = NODE($1);
								$$ = eh_addnode(T_ASSIGN, lvalue, NODE(eh_addnode(T_ADD, lvalue, Integer::make(1))));
							}
	| expression T_MINMIN	{
								ehval_p lvalue = NODE($1);
								$$ = eh_addnode(T_ASSIGN, lvalue, NODE(eh_addnode(T_SUBTRACT, lvalue, Integer::make(1))));
							}
	| expression T_CUSTOMOP expression
							{ $$ = eh_addnode(T_CUSTOMOP, NODE($1), String::make($2), NODE($3)); }
	| expression '=' expression
							{ $$ = ADD_NODE2(T_ASSIGN, $1, $3); }
	| expression T_PLUSEQ expression
							{ ADD_COMPOUND(T_ADD, $1, $3, $$); }
	| expression T_MINEQ expression
							{ ADD_COMPOUND(T_SUBTRACT, $1, $3, $$); }
	| expression T_MULTIPLYEQ expression
							{ ADD_COMPOUND(T_MULTIPLY, $1, $3, $$); }
	| expression T_DIVIDEEQ expression
							{ ADD_COMPOUND(T_DIVIDE, $1, $3, $$); }
	| expression T_MODULOEQ expression
							{ ADD_COMPOUND(T_MODULO, $1, $3, $$); }
	| expression T_ANDEQ expression
							{ ADD_COMPOUND(T_AND, $1, $3, $$); }
	| expression T_OREQ expression
							{ ADD_COMPOUND(T_OR, $1, $3, $$); }
	| expression T_XOREQ expression
							{ ADD_COMPOUND(T_XOR, $1, $3, $$); }
	| expression T_BINANDEQ expression
							{ ADD_COMPOUND(T_BINARY_AND, $1, $3, $$); }
	| expression T_BINOREQ expression
							{ ADD_COMPOUND(T_BINARY_OR, $1, $3, $$); }
	| expression T_BINXOREQ expression
							{ ADD_COMPOUND(T_BINARY_XOR, $1, $3, $$); }
	| expression T_LEFTSHIFTEQ expression
							{ ADD_COMPOUND(T_LEFTSHIFT, $1, $3, $$); }
	| expression T_RIGHTSHIFTEQ expression
							{ ADD_COMPOUND(T_RIGHTSHIFT, $1, $3, $$); }
	| expression T_ARROW expression
							{ $$ = ADD_NODE2(T_ARROW, $1, $3); }
	| expression T_DOUBLEARROW expression
							{ $$ = ADD_NODE2(T_FUNC, $1, $3); }
	| expression '.' T_VARIABLE
							{ $$ = eh_addnode(T_ACCESS, NODE($1), String::make($3)); }
	| expression ',' expression
							{
								// slight hack in order to be able to distinguish between normal and "mixed" tuples
								auto left = $1;
								auto right = $3;
								if(left->member_id == T_NAMED_ARGUMENT || right->member_id == T_NAMED_ARGUMENT || right->member_id == T_MIXED_TUPLE) {
									$$ = ADD_NODE2(T_MIXED_TUPLE, left, right);
								} else {
									$$ = ADD_NODE2(T_COMMA, left, right);
								}
							}
	| expression T_EQ expression
							{ $$ = ADD_NODE2(T_EQ, $1, $3); }
	| expression '>' expression
							{ $$ = ADD_NODE2(T_GREATER, $1, $3); }
	| expression '<' expression
							{ $$ = ADD_NODE2(T_LESSER, $1, $3); }
	| expression T_GE expression
							{ $$ = ADD_NODE2(T_GE, $1, $3); }
	| expression T_LE expression
							{ $$ = ADD_NODE2(T_LE, $1, $3); }
	| expression T_NE expression
							{ $$ = ADD_NODE2(T_NE, $1, $3); }
	| expression T_COMPARE expression
	            			{ $$ = ADD_NODE2(T_COMPARE, $1, $3); }
	| expression '+' expression
							{ $$ = ADD_NODE2(T_ADD, $1, $3); }
	| expression '-' expression
							{ $$ = ADD_NODE2(T_SUBTRACT, $1, $3); }
	| expression '*' expression
							{ $$ = ADD_NODE2(T_MULTIPLY, $1, $3); }
	| expression '/' expression
							{ $$ = ADD_NODE2(T_DIVIDE, $1, $3); }
	| expression '%' expression
							{ $$ = ADD_NODE2(T_MODULO, $1, $3); }
	| expression '^' expression
							{ $$ = ADD_NODE2(T_BINARY_XOR, $1, $3); }
	| expression '|' expression
							{ $$ = ADD_NODE2(T_BINARY_OR, $1, $3); }
	| expression '&' expression
							{ $$ = ADD_NODE2(T_BINARY_AND, $1, $3); }
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
	| expression %prec T_CALL expression
							{ $$ = ADD_NODE2(T_CALL, $1, $2); }
	| '(' '$' command ')'	{ $$ = $3; }
	| '[' arraylist ']'		{ $$ = ADD_NODE1(T_ARRAY_LITERAL, $2); }
	| '{' anonclasslist '}'	{ $$ = ADD_NODE1(T_HASH_LITERAL, $2); }
	| T_FUNC ':' parglist T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE2(T_FUNC, $3, $5); }
	| T_FUNC T_VARIABLE ':' parglist T_SEPARATOR statement_list T_END
							{
								$$ = ADD_NODE2(T_ASSIGN,
									eh_addnode(T_VARIABLE, String::make($2)),
									ADD_NODE2(T_FUNC, $4, $6)
								);
							}
	| T_CLASS T_VARIABLE T_SEPARATOR statement_list T_END
							{ $$ = eh_addnode(T_NAMED_CLASS, String::make($2), NODE($4)); }
	| T_CLASS T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_ENUM T_VARIABLE T_SEPARATOR separators enum_list T_SEPARATOR statement_list T_END
							{ $$ = eh_addnode(T_ENUM, String::make($2), NODE($5), NODE($7)); }
	| T_SWITCH expression T_SEPARATOR separators caselist T_END
							{ $$ = ADD_NODE2(T_SWITCH, $2, $5); }
	| T_GIVEN expression T_SEPARATOR separators caselist T_END
							{ $$ = ADD_NODE2(T_GIVEN, $2, $5); }
	| T_MATCH expression T_SEPARATOR separators caselist T_END
							{ $$ = ADD_NODE2(T_MATCH, $2, $5); }
	| T_IF expression T_SEPARATOR statement_list elseif_clauses T_END
							{ $$ = ADD_NODE3(T_IF, $2, $4, $5); }
	| T_IF expression T_SEPARATOR statement_list elseif_clauses T_ELSE statement_list T_END
							{ $$ = ADD_NODE4(T_IF_ELSE, $2, $4, $5, $7); }
	| T_TRY statement_list catch_clauses T_END
							{ $$ = ADD_NODE2(T_TRY, $2, $3); }
	| T_TRY statement_list catch_clauses T_FINALLY statement_list T_END
							{ $$ = ADD_NODE3(T_TRY_FINALLY, $2, $3, $5); }
	| T_WHILE expression T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE2(T_WHILE, $2, $4); }
	| T_FOR expression T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE2(T_FOR, $2, $4); }
	| T_FOR expression T_IN expression T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE3(T_FOR_IN, $2, $4, $6); }
	| attributelist expression %prec T_ATTRIBUTE
							{ $$ = ADD_NODE2(T_CLASS_MEMBER, $1, $2); }
	;

para_expr:
	/*
	 * Expression used in command arguments and array and hash literal members. Disallows tuples and function calls.
	 */
	T_INTEGER				{ $$ = eh_addnode(T_LITERAL, Integer::make($1)); }
	| T_NULL				{ $$ = ADD_NODE0(T_NULL); }
	| T_BOOL				{ $$ = eh_addnode(T_LITERAL, Bool::make($1)); }
	| T_FLOAT				{ $$ = eh_addnode(T_LITERAL, Float::make($1)); }
	| T_VARIABLE			{ $$ = eh_addnode(T_VARIABLE, String::make($1)); }
	| T_STRING				{ $$ = eh_addnode(T_LITERAL, String::make($1)); }
	| T_THIS				{ $$ = ADD_NODE0(T_THIS); }
	| T_SCOPE				{ $$ = ADD_NODE0(T_SCOPE); }
	| '_'					{ $$ = ADD_NODE0(T_ANYTHING); }
	| '(' expression ')'	{ $$ = ADD_NODE1(T_GROUPING, $2); }
	| '~' para_expr			{ $$ = ADD_NODE1(T_BINARY_COMPLEMENT, $2); }
	| '!' para_expr			{ $$ = ADD_NODE1(T_NOT, $2); }
	| para_expr T_ARROW para_expr
							{ $$ = ADD_NODE2(T_ARROW, $1, $3); }
	| para_expr '.' T_VARIABLE
							{ $$ = eh_addnode(T_ACCESS, NODE($1), String::make($3)); }
	| para_expr T_EQ para_expr
							{ $$ = ADD_NODE2(T_EQ, $1, $3); }
	| para_expr '<' para_expr
							{ $$ = ADD_NODE2(T_LESSER, $1, $3); }
	| para_expr T_GE para_expr
							{ $$ = ADD_NODE2(T_GE, $1, $3); }
	| para_expr T_LE para_expr
							{ $$ = ADD_NODE2(T_LE, $1, $3); }
	| para_expr T_NE para_expr
							{ $$ = ADD_NODE2(T_NE, $1, $3); }
	| para_expr T_COMPARE para_expr
	            			{ $$ = ADD_NODE2(T_COMPARE, $1, $3); }
	| para_expr '+' para_expr
							{ $$ = ADD_NODE2(T_ADD, $1, $3); }
	| para_expr '*' para_expr
							{ $$ = ADD_NODE2(T_MULTIPLY, $1, $3); }
	| para_expr '/' para_expr
							{ $$ = ADD_NODE2(T_DIVIDE, $1, $3); }
	| para_expr '%' para_expr
							{ $$ = ADD_NODE2(T_MODULO, $1, $3); }
	| para_expr '^' para_expr
							{ $$ = ADD_NODE2(T_BINARY_XOR, $1, $3); }
	| para_expr '|' para_expr
							{ $$ = ADD_NODE2(T_BINARY_OR, $1, $3); }
	| para_expr '&' para_expr
							{ $$ = ADD_NODE2(T_BINARY_AND, $1, $3); }
	| para_expr T_AND para_expr
							{ $$ = ADD_NODE2(T_AND, $1, $3); }
	| para_expr T_OR para_expr
							{ $$ = ADD_NODE2(T_OR, $1, $3); }
	| para_expr T_XOR para_expr
							{ $$ = ADD_NODE2(T_XOR, $1, $3); }
	| para_expr T_RANGE para_expr
							{ $$ = ADD_NODE2(T_RANGE, $1, $3); }
	| '[' arraylist ']'		{ $$ = ADD_NODE1(T_ARRAY_LITERAL, $2); }
	| T_CLASS T_SEPARATOR statement_list T_END
							{ $$ = ADD_NODE1(T_CLASS, $3); }
	| T_GIVEN expression T_SEPARATOR separators caselist T_END
							{ $$ = ADD_NODE2(T_GIVEN, $2, $5); }
	| T_MATCH expression T_SEPARATOR separators caselist T_END
							{ $$ = ADD_NODE2(T_MATCH, $2, $5); }
	| '(' '$' command ')'	{ $$ = $3; }
	| '{' anonclasslist '}'	{ $$ = ADD_NODE1(T_HASH_LITERAL, $2); }
	;

/* Commands */
command:
	T_VARIABLE paralist		{ $$ = eh_addnode(T_COMMAND, String::make($1), NODE($2)); }
	;

paralist:
	para paralist			{ $$ = ADD_NODE2(T_COMMA, $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

para:
	para_expr				{ $$ = $1; }
	| T_MINMIN bareword_or_string '=' para_expr
							{ $$ = eh_addnode(T_LONGPARA, String::make($2), NODE($4)); }
	| T_MINMIN bareword_or_string
							{ $$ = eh_addnode(T_LONGPARA, String::make($2), Bool::make(true)); }
	| '-' bareword_or_string
							{ $$ = eh_addnode(T_SHORTPARA, String::make($2), Bool::make(true)); }
	| '-' bareword_or_string '=' para_expr
							{ $$ = eh_addnode(T_SHORTPARA, String::make($2), NODE($4)); }
	;

bareword_or_string:
	T_VARIABLE				{ $$ = $1; }
	| T_STRING				{ $$ = $1; }

/* If statements */
elseif_clauses:
	elseif_clause elseif_clauses
							{ $$ = ADD_NODE2(T_COMMA, $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

elseif_clause:
	T_ELSIF expression T_SEPARATOR statement_list
							{ $$ = ADD_NODE2(T_ELSIF, $2, $4); }
	;

/* Try-catch */
catch_clauses:
	catch_clause catch_clauses
							{ $$ = ADD_NODE2(T_COMMA, $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

catch_clause:
	T_CATCH T_SEPARATOR statement_list
							{ $$ = ADD_NODE1(T_CATCH, $3); }
	| T_CATCH T_IF expression T_SEPARATOR statement_list
							{ $$ = ADD_NODE2(T_CATCH_IF, $3, $5); }
	;

/* Array literals */
arraylist:
	arraylist_i arraymember ','
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| arraylist_i arraymember
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

arraylist_i:
	arraylist_i arraymember ','
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

arraymember:
	para_expr T_DOUBLEARROW para_expr
							{ $$ = ADD_NODE2(T_ARRAY_MEMBER, $1, $3); }
	| para_expr				{ $$ = ADD_NODE1(T_ARRAY_MEMBER_NO_KEY, $1); }
	;

/* Hash literals */
anonclasslist:
	anonclasslist_i anonclassmember ','
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| anonclasslist_i anonclassmember
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

anonclasslist_i:
	anonclasslist_i anonclassmember ','
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

anonclassmember:
	T_VARIABLE ':' para_expr
							{ $$ = eh_addnode(T_ARRAY_MEMBER, String::make($1), NODE($3)); }
	| T_STRING ':' para_expr
							{ $$ = eh_addnode(T_ARRAY_MEMBER, String::make($1), NODE($3)); }
	;

parglist:
	expression				{ $$ = $1; }
	| /* NULL */			{ $$ = ADD_NODE0(T_NULL); }
	;

/* Switch etcetera */
caselist:
	acase caselist			{ $$ = ADD_NODE2(T_COMMA, $1, $2); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

acase:
	T_CASE expression T_SEPARATOR statement_list
							{ $$ = ADD_NODE2(T_CASE, $2, $4); }
	| T_DEFAULT T_SEPARATOR statement_list
							{ $$ = ADD_NODE1(T_DEFAULT, $3); }
	;

separators:
	T_SEPARATOR separators	{ }
	| /* NULL */			{ }

/* Property declarations */
attributelist:
	T_ATTRIBUTE attributelist_inner
							{ $$ = eh_addnode(T_ATTRIBUTE, Attribute::make($1, PARENT), NODE($2)); }

attributelist_inner:
	T_ATTRIBUTE attributelist_inner
							{ $$ = eh_addnode(T_ATTRIBUTE, Attribute::make($1, PARENT), NODE($2)); }
	| /* NULL */ %prec '='	{ $$ = ADD_NODE0(T_END); }
	;

/* Enums */
enum_list:
	enum_member				{ $$ = $1; }
	| enum_list ',' enum_member
							{ $$ = ADD_NODE2(T_COMMA, $3, $1); }
	;

enum_member:
	T_VARIABLE				{ $$ = eh_addnode(T_NULLARY_ENUM, String::make($1)); }
	| T_VARIABLE '(' enum_arg_list ')'
							{ $$ = eh_addnode(T_ENUM_WITH_ARGUMENTS, String::make($1), NODE($3)); }
	;

enum_arg_list:
	T_VARIABLE				{ $$ = eh_addnode(T_LITERAL, String::make($1)); }
	| T_VARIABLE ',' enum_arg_list
							{ $$ = eh_addnode(T_COMMA, String::make($1), NODE($3)); }
	;
%%
