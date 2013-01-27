%{
#include "../std_lib_includes.hpp"
#include "json.hpp"
#include "eh_json.flex.hpp"

#define PARENT eh_json_get_extra(scanner)->ehi->get_parent()

#define YYLEX_PARAM scanner

#define YYERROR_VERBOSE

void eh_json_error(void *scanner, const char *message) [[noreturn]];
%}
%pure-parser
%parse-param { void *scanner }
%name-prefix="eh_json_"

%token T_NULL
%token T_BOOL
%token T_INTEGER
%token T_FLOAT
%token T_STRING
%token ',' '[' ']' '{' '}' ':'
%%
program:
	value			{
						eh_json_get_extra(scanner)->result = $1;
					}
;;

value:
	T_NULL			{ $$ = $1; }
	| T_BOOL		{ $$ = $1; }
	| T_INTEGER		{ $$ = $1; }
	| T_FLOAT		{ $$ = $1; }
	| T_STRING		{ $$ = $1; }
	| '{' '}'		{ $$ = Hash::make(PARENT); }
	| '[' ']'		{ $$ = Array::make(PARENT); }
	| '{' hash_content T_STRING ':' value '}'
					{
						ehval_p hash = $2;
						hash->get<Hash>()->set($3->get<String>(), $5);
						$$ = hash;
					}
	| '[' array_content value ']'
	 				{
	 					ehval_p array = $2;
	 					int index = array->get<Array>()->size();
	 					array->get<Array>()->int_indices[index] = $3;
	 					$$ = array;
	 				}
;;

hash_content:
	hash_content T_STRING ':' value ','
					{
						ehval_p hash = $1;
						hash->get<Hash>()->set($2->get<String>(), $4);
						$$ = hash;
					}
	| /* NULL */	{ $$ = Hash::make(PARENT); }
;;

array_content:
	array_content value ','
	 				{
	 					ehval_p array = $1;
	 					int index = array->get<Array>()->size();
	 					array->get<Array>()->int_indices[index] = $2;
	 					$$ = array;
	 				}
	| /* NULL */	{ $$ = Array::make(PARENT); }
%%
