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
#define OPERATOR(name) (String::make(strdup(("operator" + std::string(name)).c_str())))
#define OPERATOR_CALL(left, op, right) (eh_addnode(T_CALL_METHOD, NODE(left), OPERATOR(op), NODE(right)))

Node *make_lines(Node *l, Node *r, void *scanner);

void yyerror(void *, const char *s);

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
%token T_WHEN
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
%token T_GENERATOR
%token T_DO
%token T_RET
%token T_SEPARATOR
%token T_NULL
%token T_ENUM
%token T_CLASS
%token T_CLASS_MEMBER
%token T_YIELD
%token T_LITERAL
%token T_TRY
%token T_CATCH
%token T_FINALLY
%token <vValue> T_ATTRIBUTE
%token T_ARRAY_MEMBER
%token T_DOUBLEARROW
%token T_CALL_METHOD T_TRY_FINALLY T_CATCH_IF T_FOR_IN T_NAMED_CLASS T_IF_ELSE T_NULLARY_ENUM T_ENUM_WITH_ARGUMENTS
%token T_ARRAY_MEMBER_NO_KEY T_ANYTHING T_GROUPING T_ASSIGN T_ADD T_SUBTRACT T_MULTIPLY T_DIVIDE T_MODULO T_GREATER
%token T_BAR T_BINARY_COMPLEMENT T_NOT T_MATCH_SET T_COMMA T_ARRAY_LITERAL
%token T_HASH_LITERAL T_CALL T_ACCESS T_LIST T_NAMED_ARGUMENT T_MIXED_TUPLE T_INSTANCE_ACCESS
%token T_COMMAND T_SHORTPARA T_LONGPARA
%token <sValue> T_VARIABLE
%token <sValue> T_STRING
%token <sValue> T_BAR_OP T_CARET_OP T_AMPERSAND_OP T_COMPARE_OP T_EQ_OP T_COMPOUND_ASSIGN T_COLON_OP T_PLUS_OP T_MULT_OP T_CUSTOM_OP T_SHIFT_OP
%token T_OR T_SCOPE T_MINMIN T_XOR T_THIS T_RAW T_ARROW T_AND T_PLUSPLUS T_RANGE
%right T_COMPOUND_ASSIGN '='
%right T_DOUBLEARROW
%nonassoc T_CLASS_MEMBER
%right ','
%nonassoc ':'
%left T_AND T_OR T_XOR
%left T_BAR_OP '|'
%left T_CARET_OP
%left T_AMPERSAND_OP
%left T_COMPARE_OP
%left T_EQ_OP
%right T_COLON_OP
%left T_SHIFT_OP
%left T_PLUS_OP '-'
%left T_MULT_OP
%left T_CUSTOM_OP
%nonassoc T_PLUSPLUS T_MINMIN
%nonassoc '~' '!'
%left T_RANGE T_ARROW '.'
%nonassoc '[' ']' '{' '}'
%nonassoc '(' ')'
%nonassoc T_INTEGER T_FLOAT T_NULL T_BOOL T_VARIABLE T_STRING T_GIVEN T_MATCH T_SWITCH T_FUNC T_DO T_CLASS T_ENUM T_IF T_TRY T_FOR T_WHILE T_THIS T_SCOPE '_'

%type<ehNode> statement expression statement_list parglist arraylist arraylist_i anonclasslist anonclassmember
%type<ehNode> anonclasslist_i attributelist attributelist_inner caselist acase command paralist para global_list
%type<ehNode> catch_clauses catch_clause
%type<ehNode> elseif_clauses elseif_clause enum_list enum_member enum_arg_list
%type<ehNode> assign_expression function_expression attribute_expression tuple_expression namedvar_expression
%type<ehNode> boolean_expression bar_expression caret_expression ampersand_expression compare_expression
%type<ehNode> colon_expression equals_expression plus_expression mult_expression shift_expression custom_op_expression
%type<ehNode> incdec_expression unary_op_expression access_expression base_expression
%type<sValue> bareword_or_string

%start program
%%
program:
	global_list				{
								EHI *ehi = yyget_extra(scanner);
								if(ehi->get_interactivity() == end_is_end_e) {
									ehval_p code = NODE($1);
									ehi->set_code(code);
								}
							}
	;

global_list:
	/* NULL */				{ $$ = nullptr; }
	| statement				{
								EHI *ehi = yyget_extra(scanner);
								if(ehi->get_interactivity() != end_is_end_e) {
									ehval_p statement = NODE($1);
									ehval_p ret = ehi->eh_execute(statement, ehi->get_context());
									std::cout << "=> " << ehi->toString(ret, ehi->get_context())->get<String>() << std::endl;
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
							{ $$ = make_lines($1, $2, scanner); }
	;

statement:
	T_SEPARATOR				{ $$ = nullptr; }
	| expression T_SEPARATOR
							{ $$ = $1; }
	;

expression:
	assign_expression		{ $$ = $1; }
	| '$' command			{ $$ = $2; }
	| T_RET expression
							{ $$ = ADD_NODE1(T_RET, $2); }
	| T_RET					{ $$ = eh_addnode(T_RET, Null::make()); }
	| T_CONTINUE			{ $$ = eh_addnode(T_CONTINUE, Integer::make(1)); }
	| T_CONTINUE expression	{ $$ = ADD_NODE1(T_CONTINUE, $2); }
	| T_BREAK 				{ $$ = eh_addnode(T_BREAK, Integer::make(1)); }
	| T_BREAK expression	{ $$ = ADD_NODE1(T_BREAK, $2); }
	;

assign_expression:
	function_expression '=' assign_expression
							{ $$ = ADD_NODE2(T_ASSIGN, $1, $3); }
	| function_expression T_COMPOUND_ASSIGN assign_expression
							{
								ehval_p lvalue = NODE($1);
								ehval_p rvalue = NODE(eh_addnode(T_CALL_METHOD, lvalue, OPERATOR($2), NODE($3)));
								$$ = eh_addnode(T_ASSIGN, lvalue, rvalue);
							}
	| function_expression	{ $$ = $1; }
	;

function_expression:
	attribute_expression T_DOUBLEARROW function_expression
							{ $$ = ADD_NODE2(T_FUNC, $1, $3); }
	| attribute_expression	{ $$ = $1; }
	;

attribute_expression:
	attributelist tuple_expression %prec T_CLASS_MEMBER
							{ $$ = ADD_NODE2(T_CLASS_MEMBER, $1, $2); }
	| tuple_expression		{ $$ = $1; }
	;

tuple_expression:
	namedvar_expression ',' separators tuple_expression
							{
								// slight hack in order to be able to distinguish between normal and "mixed" tuples
								auto left = $1;
								auto right = $4;
								if(left->member_id == T_NAMED_ARGUMENT || right->member_id == T_NAMED_ARGUMENT || right->member_id == T_MIXED_TUPLE) {
									$$ = ADD_NODE2(T_MIXED_TUPLE, left, right);
								} else {
									$$ = ADD_NODE2(T_COMMA, left, right);
								}
							}
	| namedvar_expression	{ $$ = $1; }
	;

namedvar_expression:
	T_VARIABLE ':' boolean_expression %prec ':'
							{ $$ = eh_addnode(T_NAMED_ARGUMENT, String::make($1), NODE($3)); }
	| boolean_expression	{ $$ = $1; }
	;

boolean_expression:
	boolean_expression T_AND bar_expression
							{ $$ = ADD_NODE2(T_AND, $1, $3); }
	| boolean_expression T_OR bar_expression
							{ $$ = ADD_NODE2(T_OR, $1, $3); }
	| boolean_expression T_XOR bar_expression
							{ $$ = ADD_NODE2(T_XOR, $1, $3); }
	| bar_expression		{ $$ = $1; }
	;

bar_expression:
	bar_expression T_BAR_OP caret_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| bar_expression '|' caret_expression
							{ $$ = ADD_NODE2(T_BAR, $1, $3); }
	| caret_expression		{ $$ = $1; }
	;

caret_expression:
	caret_expression T_CARET_OP ampersand_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| ampersand_expression	{ $$ = $1; }
	;

ampersand_expression:
	ampersand_expression T_AMPERSAND_OP compare_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| compare_expression	{ $$ = $1; }
	;

compare_expression:
	compare_expression T_COMPARE_OP equals_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| equals_expression		{ $$ = $1; }
	;

equals_expression:
	equals_expression T_EQ_OP colon_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| colon_expression		{ $$ = $1; }
	;

colon_expression:
	plus_expression T_COLON_OP colon_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| plus_expression		{ $$ = $1; }
	;

plus_expression:
	plus_expression T_PLUS_OP mult_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| plus_expression '-' mult_expression
							{ $$ = eh_addnode(T_CALL_METHOD, NODE($1), String::make(strdup("operator-")), NODE($3)); }
	| mult_expression		{ $$ = $1; }
	;

mult_expression:
	mult_expression T_MULT_OP shift_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| shift_expression		{ $$ = $1; }
	;

shift_expression:
	shift_expression T_SHIFT_OP custom_op_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| custom_op_expression	{ $$ = $1; }
	;

custom_op_expression:
	custom_op_expression T_CUSTOM_OP incdec_expression
							{ $$ = OPERATOR_CALL($1, $2, $3); }
	| incdec_expression		{ $$ = $1; }
	;

incdec_expression:
	incdec_expression T_PLUSPLUS
							{
								ehval_p lvalue = NODE($1);
								ehval_p rvalue = NODE(eh_addnode(T_CALL_METHOD, lvalue, String::make(strdup("operator+")), Integer::make(1)));
								$$ = eh_addnode(T_ASSIGN, lvalue, rvalue);
							}
	| incdec_expression T_MINMIN
							{
								ehval_p lvalue = NODE($1);
								ehval_p rvalue = NODE(eh_addnode(T_CALL_METHOD, lvalue, String::make(strdup("operator-")), Integer::make(1)));
								$$ = eh_addnode(T_ASSIGN, lvalue, rvalue);
							}
	| access_expression T_AS T_VARIABLE
							{ $$ = eh_addnode(T_AS, NODE($1), String::make($3)); }
	| access_expression		{ $$ = $1; }
	;

access_expression:
	access_expression T_ARROW unary_op_expression
							{ $$ = ADD_NODE2(T_ARROW, $1, $3); }
	| access_expression '.' T_VARIABLE
							{ $$ = eh_addnode(T_ACCESS, NODE($1), String::make($3)); }
	| access_expression T_INSTANCE_ACCESS T_VARIABLE
							{ $$ = eh_addnode(T_INSTANCE_ACCESS, NODE($1), String::make($3)); }
	| unary_op_expression T_RANGE unary_op_expression
							{ $$ = ADD_NODE2(T_RANGE, $1, $3); }
	| access_expression	unary_op_expression
							{ $$ = ADD_NODE2(T_CALL, $1, $2); }
	| T_YIELD unary_op_expression
							{ $$ = ADD_NODE1(T_YIELD, $2); }
	| unary_op_expression	{ $$ = $1; }
	;

unary_op_expression:
	'~' unary_op_expression	{ $$ = ADD_NODE1(T_BINARY_COMPLEMENT, $2); }
	| '!' unary_op_expression
							{ $$ = ADD_NODE1(T_NOT, $2); }
	| T_RAW unary_op_expression
							{ $$ = ADD_NODE1(T_RAW, $2); }
	| base_expression		{ $$ = $1; }
	;

base_expression:
	T_INTEGER				{ $$ = eh_addnode(T_LITERAL, Integer::make($1)); }
	| T_NULL				{ $$ = ADD_NODE0(T_NULL); }
	| T_BOOL				{ $$ = eh_addnode(T_LITERAL, Bool::make($1)); }
	| T_FLOAT				{ $$ = eh_addnode(T_LITERAL, Float::make($1)); }
	| T_VARIABLE			{ $$ = eh_addnode(T_VARIABLE, String::make($1)); }
	| T_STRING				{ $$ = eh_addnode(T_LITERAL, String::make($1)); }
	| T_THIS				{ $$ = ADD_NODE0(T_THIS); }
	| T_SCOPE				{ $$ = ADD_NODE0(T_SCOPE); }
	| '_'					{ $$ = ADD_NODE0(T_ANYTHING); }
	| '@' T_VARIABLE		{ $$ = eh_addnode(T_MATCH_SET, String::make($2)); }
	| '(' separators expression separators ')'	{ $$ = ADD_NODE1(T_GROUPING, $3); }
	| '[' separators arraylist ']'		{ $$ = ADD_NODE1(T_ARRAY_LITERAL, $3); }
	| '{' separators anonclasslist '}'	{ $$ = ADD_NODE1(T_HASH_LITERAL, $3); }
	| T_DO statement_list T_END
							{ $$ = $2; }
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
	base_expression			{ $$ = $1; }
	| T_MINMIN bareword_or_string '=' base_expression
							{ $$ = eh_addnode(T_LONGPARA, String::make($2), NODE($4)); }
	| T_MINMIN bareword_or_string
							{ $$ = eh_addnode(T_LONGPARA, String::make($2), Bool::make(true)); }
	| '-' bareword_or_string
							{ $$ = eh_addnode(T_SHORTPARA, String::make($2), Bool::make(true)); }
	| '-' bareword_or_string '=' base_expression
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
	arraylist_i namedvar_expression ',' separators
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| arraylist_i namedvar_expression separators
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

arraylist_i:
	arraylist_i namedvar_expression ',' separators
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

/* Hash literals */
anonclasslist:
	anonclasslist_i anonclassmember ',' separators
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| anonclasslist_i anonclassmember separators
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

anonclasslist_i:
	anonclasslist_i anonclassmember ',' separators
							{ $$ = ADD_NODE2(T_COMMA, $2, $1); }
	| /* NULL */			{ $$ = ADD_NODE0(T_END); }
	;

anonclassmember:
	T_VARIABLE ':' namedvar_expression
							{ $$ = eh_addnode(T_ARRAY_MEMBER, String::make($1), NODE($3)); }
	| T_STRING ':' namedvar_expression
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
	| T_CASE expression T_WHEN expression T_SEPARATOR statement_list
							{ $$ = ADD_NODE3(T_WHEN, $2, $4, $6); }
	| T_DEFAULT T_SEPARATOR statement_list
							{ $$ = ADD_NODE1(T_DEFAULT, $3); }
	;

separators:
	separators T_SEPARATOR	{ }
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
	| enum_member ',' separators enum_list
							{ $$ = ADD_NODE2(T_COMMA, $1, $4); }
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

const char *get_token_name(int token) {
	return yytname[token - 255];
}

Node *make_lines(Node *l, Node *r, void *scanner) {
	// get rid of empty lines
	if(l == nullptr) {
		return r;
	} else if(r == nullptr) {
		return l;
	} else {
		return ADD_NODE2(T_SEPARATOR, l, r);
	}
}
