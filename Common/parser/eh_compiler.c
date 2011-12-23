#include "eh.h"

// file to write to
FILE *outfile;
// label used
unsigned int label = 0;
static int compile(ehnode_t *node);

int execute(ehnode_t *node) {
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
}

static int compile(ehnode_t *node) {
	if(node == NULL)
		return 0;
	//printf("Executing nodetype %d\n", node->type);
	switch(node->type) {
		/* Not sure yet how to handle strings
		case idnode_enum:
			fprintf(outfile, 
			return node->id.name;
		*/
		case connode_enum:
			fprintf(outfile, "pushl $%d\n", node->con.value);
			return 0;
		case opnode_enum:
			//printf("Executing opcode: %d\n", node->op.op);
			switch(node->op.op) {
				case T_ECHO:
					switch(node->op.paras[0]->type) {
						case idnode_enum:
						case opnode_enum:
							// make sure stack is aligned
							fprintf(outfile, "subl $4, %%esp\n");
							compile(node->op.paras[0]);
							// argument will already be on top of stack 
							fprintf(outfile, "pushl $printfnum\n");
							fprintf(outfile, "call _printf\n");
							fprintf(outfile, "addl $8, %%esp\n");
							return 0;
						case connode_enum:
							printf("%d\n", node->op.paras[0]->con.value);
							return 0;
					}
					
					return 0;
				case T_IF:
					// we'll need a label
					label++;
					// compile the condition
					compile(node->op.paras[0]);
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "cmpl $0, %%eax\n");
					fprintf(outfile, "je L%03d\n", label);
					// compile the then
					compile(node->op.paras[1]);
					fprintf(outfile, "L%03d:\n", label);
					return 0;
				case T_SEPARATOR:
					compile(node->op.paras[0]);
					// discard return value
					fprintf(outfile, "popl %%eax\n");
					compile(node->op.paras[1]);
					fprintf(outfile, "popl %%eax\n");
					return 0;
				case '=':
					compile(node->op.paras[0]);
					compile(node->op.paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "xorl %%eax, %%eax\n");
					// apparently, cmove can't have an immediate as its source
					fprintf(outfile, "movl $1, %%edx\n");
					fprintf(outfile, "cmpl %%ecx, %%ebx\n");
					fprintf(outfile, "cmove %%edx, %%eax\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
/*				case '>':
					return execute(node->op.paras[0]) >
						execute(node->op.paras[1]);
				case '<':
					return execute(node->op.paras[0]) <
						execute(node->op.paras[1]);
				case T_GE:
					return execute(node->op.paras[0]) >= 
						execute(node->op.paras[1]);
				case T_LE:
					return execute(node->op.paras[0]) <=
						execute(node->op.paras[1]);
				case T_NE:
					return execute(node->op.paras[0]) != 
						execute(node->op.paras[1]);*/
				case '+':
					compile(node->op.paras[0]);
					compile(node->op.paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "addl %%ecx, %%ebx\n");
					fprintf(outfile, "pushl %%ebx\n");
					break;
				case '-':
					compile(node->op.paras[0]);
					compile(node->op.paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "subl %%ecx, %%ebx\n");
					fprintf(outfile, "pushl %%ebx\n");
					break;
				case '*':
					compile(node->op.paras[0]);
					compile(node->op.paras[1]);
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "imull %%ebx\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				case '/':
					compile(node->op.paras[0]);
					compile(node->op.paras[1]);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "xorl %%edx, %%edx\n");
					fprintf(outfile, "idivl %%ecx\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				default:
					printf("Unexpected opcode %d\n", node->op.op);
					exit(0);
			}
	}
	return 0;
}
