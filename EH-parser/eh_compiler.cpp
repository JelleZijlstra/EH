/*
 * eh_compiler.c
 * Jelle Zijlstra, December 2011
 *
 * A minimal compiler for the EH language. Does not support any useful feature
 * of the language, really.
 */
#include "eh.h"
#include "eh.bison.hpp"

// file to write to
FILE *outfile;
// label used
unsigned int label = 0;
static int compile(ehretval_t *node);

void eh_init(void) {
	return;
}
void eh_exit(void) {
	return;
}

ehretval_t execute(ehretval_t *node, ehcontext_t context) {
	outfile = fopen("tmp.s", "w");
	if(outfile == NULL) {
		fprintf(stderr, "Unable to create output file");
		exit(1);
	}
	// prepare for printf output
	fprintf(outfile, ".data\n");
	fprintf(outfile, "printfnum:\n");
	fprintf(outfile, ".asciz \"%%d\\n\"\n");
	fprintf(outfile, ".text\n");
	fprintf(outfile, ".globl _main\n");
	fprintf(outfile, "_main:\n");
	compile(node);
	// need to pad stack so esp is 16-byte aligned
	fprintf(outfile, "movl $0xbffff770, %%esp\n");
	fprintf(outfile, "call _exit\n");

	// we need to return a retval_t, so do it
	ehretval_t ret;
	ret.type = int_e;
	ret.intval = 0;
	return ret;
}

static int compile(ehretval_t *node) {
	if(node == NULL)
		return 0;
	//printf("Executing nodetype %d\n", node->type);
	switch(node->type) {
		/* Not sure yet how to handle strings
		case string_e:
			fprintf(outfile,
			return node->id.name;
		*/
		case int_e:
			fprintf(outfile, "pushl $%d\n", node->intval);
			return 0;
		case op_e:
			//printf("Executing opcode: %d\n", node->opval->op);
			switch(node->opval->op) {
				case T_ECHO:
					switch(node->opval->paras[0]->type) {
						case int_e:
						case op_e:
							// make sure stack is aligned
							fprintf(outfile, "subl $4, %%esp\n");
							compile(node->opval->paras[0]);
							// argument will already be on top of stack
							fprintf(outfile, "pushl $printfnum\n");
							fprintf(outfile, "call _printf\n");
							fprintf(outfile, "addl $8, %%esp\n");
							return 0;
						case string_e:
							printf("Constant %d\n", node->opval->paras[0]->intval);
							return 0;
						default:
							fprintf(stderr, "Unsupported argument for echo\n");
							break;
					}

					return 0;
				case T_IF:
					// we'll need a label
					label++;
					// compile the condition
					compile(node->opval->paras[0]);
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "cmpl $0, %%eax\n");
					fprintf(outfile, "je L%03d\n", label);
					// compile the then
					compile(node->opval->paras[1]);
					fprintf(outfile, "L%03d:\n", label);
					return 0;
				case T_WHILE:
					// we'll need two labels
					label++;
					label++;
					// first label
					fprintf(outfile, "L%03d:\n", label - 1);
					// compile the condition
					compile(node->opval->paras[0]);
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "cmpl $0, %%eax\n");
					fprintf(outfile, "je L%03d\n", label);
					// compile the then
					compile(node->opval->paras[1]);
					// jump back to condition
					fprintf(outfile, "jmp L%03d\n", label - 1);
					fprintf(outfile, "L%03d:\n", label);
					break;
				case T_SEPARATOR:
					compile(node->opval->paras[0]);
					// discard return value
					fprintf(outfile, "popl %%eax\n");
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%eax\n");
					return 0;
				case '=':
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "xorl %%eax, %%eax\n");
					// apparently, cmove can't have an immediate as its source
					fprintf(outfile, "movl $1, %%edx\n");
					fprintf(outfile, "cmpl %%ecx, %%ebx\n");
					fprintf(outfile, "cmove %%edx, %%eax\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				case '>':
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "xorl %%eax, %%eax\n");
					// apparently, cmove can't have an immediate as its source
					fprintf(outfile, "movl $1, %%edx\n");
					fprintf(outfile, "cmpl %%ecx, %%ebx\n");
					fprintf(outfile, "cmovg %%edx, %%eax\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				case '<':
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "xorl %%eax, %%eax\n");
					// apparently, cmove can't have an immediate as its source
					fprintf(outfile, "movl $1, %%edx\n");
					fprintf(outfile, "cmpl %%ecx, %%ebx\n");
					fprintf(outfile, "cmovl %%edx, %%eax\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				case T_GE:
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "xorl %%eax, %%eax\n");
					// apparently, cmove can't have an immediate as its source
					fprintf(outfile, "movl $1, %%edx\n");
					fprintf(outfile, "cmpl %%ecx, %%ebx\n");
					fprintf(outfile, "cmovge %%edx, %%eax\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				case T_LE:
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "xorl %%eax, %%eax\n");
					// apparently, cmove can't have an immediate as its source
					fprintf(outfile, "movl $1, %%edx\n");
					fprintf(outfile, "cmpl %%ecx, %%ebx\n");
					fprintf(outfile, "cmovle %%edx, %%eax\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				case T_NE:
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "xorl %%eax, %%eax\n");
					// apparently, cmove can't have an immediate as its source
					fprintf(outfile, "movl $1, %%edx\n");
					fprintf(outfile, "cmpl %%ecx, %%ebx\n");
					fprintf(outfile, "cmovne %%edx, %%eax\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				case '+':
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "addl %%ecx, %%ebx\n");
					fprintf(outfile, "pushl %%ebx\n");
					break;
				case '-':
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "subl %%ecx, %%ebx\n");
					fprintf(outfile, "pushl %%ebx\n");
					break;
				case '*':
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "imull %%ebx\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				case '/':
					compile(node->opval->paras[0]);
					compile(node->opval->paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "xorl %%edx, %%edx\n");
					fprintf(outfile, "idivl %%ecx\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				default:
					printf("Unexpected opcode %d\n", node->opval->op);
					exit(0);
			}
			break;
		default:
			fprintf(stderr, "I'm too stupid a compiler; I can't handle this (type %d)\n", node->type);
			break;
	}
	return 0;
}

void eh_setarg(int argc, char **argv) {
	// how should we do this?
	return;
}