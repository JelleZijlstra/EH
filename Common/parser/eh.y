/* This code was inspired by Tom Niemann's "A Compact Guide to Lex & Yacc", available at http://epaperpress.com/lexandyacc/ */
%{
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
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
%token T_ECHO
%token T_SEPARATOR
%token <sValue> T_STRING
%left '='
%left '+' '-'
%left '*' '/'

%type<ehNode> statement expression statement_list string
%%
program:
	statement_list			{ execute($1); exit(0); }
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
	| T_SEPARATOR			{ $$ = 0; }
	;

expression:
	T_INTEGER				{ $$ = get_constant($1); }
	| expression '=' expression 
							{ $$ = operate('=', 2, $1, $3); }
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
ehnode_t *get_constant(int value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));

	ret->type = connode_enum;
	ret->con.value = value;
	
	//printf("Returning constant %d\n", ret->con.value);
	return ret;
}
ehnode_t *get_identifier(char *value) {
	ehnode_t *ret;
	ret = Malloc(sizeof(ehnode_t));
	
	ret->type = idnode_enum;
	ret->id.name = Malloc(strlen(value) + 1);
	strcpy(ret->id.name, value);
	
	//printf("Returning identifier %s\n", ret->id.name);
	return ret;
}
ehnode_t *operate(int operation, int nparas, ...) {
	va_list args;
	ehnode_t *ret;
	int i;
	
	ret = Malloc(sizeof(ehnode_t));
	ret->op.paras = Malloc(nparas * sizeof(ehnode_t *));
	
	ret->type = opnode_enum;
	ret->op.op = operation;
	ret->op.nparas = nparas;
	//printf("Adding operation %d with %d paras\n", ret->op.op, ret->op.nparas);
	va_start(args, nparas);
	for(i = 0; i < nparas; i++) {
		ret->op.paras[i] = va_arg(args, ehnode_t *);
		//printf("Para %d is of type %d\n", i, ret->op.paras[i]->type);
	}
	va_end(args);
	
	return ret;
}
int execute(ehnode_t *node) {
	if(node == NULL)
		return 0;
	//printf("Executing nodetype %d\n", node->type);
	switch(node->type) {
		case idnode_enum:
			return node->id.name;
		case connode_enum:
			return node->con.value;
		case opnode_enum:
			//printf("Executing opcode: %d\n", node->op.op);
			switch(node->op.op) {
				case T_ECHO:
					switch(node->op.paras[0]->type) {
						case idnode_enum:
							printf("%s\n", node->op.paras[0]->id.name);
							return 0;
						case connode_enum:
							printf("%d\n", node->op.paras[0]->con.value);
							return 0;
						case opnode_enum:
							printf("%d\n", execute(node->op.paras[0]));
					}
					
					return 0;
				case T_IF:
					if(execute(node->op.paras[0]))
						execute(node->op.paras[1]);
					return 0;
				case T_SEPARATOR:
					execute(node->op.paras[0]);
					execute(node->op.paras[1]);
					return 0;
				case '=':
					return execute(node->op.paras[0]) == 
						execute(node->op.paras[1]);
				case '+':
					return execute(node->op.paras[0]) + 
						execute(node->op.paras[1]);
				case '-':
					return execute(node->op.paras[0]) - 
						execute(node->op.paras[1]);
				case '*':
					return execute(node->op.paras[0]) * 
						execute(node->op.paras[1]);
				case '/':
					return execute(node->op.paras[0]) / 
						execute(node->op.paras[1]);
				default:
					printf("Unexpected opcode %d\n", node->op.op);
					exit(0);
			}
	}
	return 0;
}
void yyerror(char *s) {
	fprintf(stderr, "%s\n", s);
}
int main(void) {
	yyparse();
	return 0;
}
void *Malloc(size_t size) {
	void *ret;
	ret = malloc(size);
	if(ret == NULL) {
		yyerror("Unable to allocate memory");
		exit(1);
	}
}