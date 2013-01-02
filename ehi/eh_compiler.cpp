/*
 * eh_compiler.c
 * Jelle Zijlstra, December 2011
 *
 * A minimal compiler for the EH language. Does not support any useful feature
 * of the language, really.
 */
#include "eh.hpp"
#include "eh.bison.hpp"

// file to write to
char *outfilename;
FILE *outfile;
// label used
unsigned int label = 0;
bool returning = false;

void usage(char **argv) {
	fprintf(stderr, "Usage: %s file -o output\n\t%s -o output file", argv[0], argv[0]);
	exit(1);
}

int main(int argc, char **argv) {
	ehval_p ret;
	char *infilename;

	// parse arguments
	if(argc < 2 || argc == 3 || argc > 4) {
		usage(argv);
	}
	// no explicit output file given: s/\..*$/.s/
	if(argc == 2) {
		infilename = argv[1];
		// default: replace .whatever with .s
		outfilename = strdup(infilename);
		if(!outfilename) {
			exit(1);
		}
		int i = strlen(outfilename) - 1;
		while(1) {
			if(i == 0) {
				// fall back to tmp.s
				free(outfilename);
				outfilename = strdup("tmp.s");
				break;
			}
			if(outfilename[i] == '.') {
				outfilename[i+1] = 's';
				outfilename[i+2] = '\0';
				break;
			}
			i--;
		}
	} else {
		// explicit output file given
		if(!strcmp(argv[1], "-o")) {
			outfilename = argv[2];
			infilename = argv[3];
		} else if(!strcmp(argv[2], "-o")) {
			infilename = argv[1];
			outfilename = argv[3];
		} else {
			usage(argv);
		}
	}

	try {
		EHI interpreter;
		interpreter.eh_setarg(argc, argv);
		// set input
		ret = interpreter.parse_file(infilename);
		interpreter.eh_exit();
		exit(ret->get<Integer>());
	}
	catch(...) {
		exit(1);
	}
	return 0;
}

void EHI::eh_init(void) {
	outfile = fopen(outfilename, "w");
	if(outfile == nullptr) {
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
	return;
}
void EHI::eh_exit(void) {
	// need to pad stack so esp is 16-byte aligned
	fprintf(outfile, "movl $0xbffff770, %%esp\n");
	fprintf(outfile, "call _exit\n");
	return;
}

ehval_p EHI::eh_execute(ehval_p node, const ehcontext_t context) {
	ehval_p ret;
	if(node == nullptr) {
		ret = Integer::make(0);
	//printf("Executing nodetype %d\n", node->type);
	} else switch(node->type()) {
		/* Not sure yet how to handle strings
		case string_e:
			fprintf(outfile,
			return node->id.name;
		*/
		case int_e:
			fprintf(outfile, "pushl $%d\n", node->get<Integer>());
			break;
		case op_e:
			//printf("Executing opcode: %d\n", node->get<Node>()->op);
			switch(node->get<Node>()->op) {
				case T_COMMAND:
					if(node->get<Node>()->paras[0]->type() == string_e
						&& !strcmp(node->get<Node>()->paras[0]->get<String>(), "echo")) {
						node = node->get<Node>()->paras[1];
						if(node->get<Node>()->nparas != 2) {
							break;
						}
						switch(node->get<Node>()->paras[0]->type()) {
							case int_e:
							case op_e:
								// make sure stack is aligned
								fprintf(outfile, "subl $4, %%esp\n");
								eh_execute(node->get<Node>()->paras[0], context);
								// argument will already be on top of stack
								fprintf(outfile, "pushl $printfnum\n");
								fprintf(outfile, "call _printf\n");
								fprintf(outfile, "addl $8, %%esp\n");
								break;
							case string_e:
								printf("Constant %d\n", node->get<Node>()->paras[0]->get<Integer>());
								break;
							default:
								fprintf(stderr, "Unsupported argument for echo\n");
								break;
						}
					}
					break;
				case T_IF:
					// we'll need a label
					label++;
					// compile the condition
					eh_execute(node->get<Node>()->paras[0], context);
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "cmpl $0, %%eax\n");
					fprintf(outfile, "je L%03d\n", label);
					// compile the then
					eh_execute(node->get<Node>()->paras[1], context);
					fprintf(outfile, "L%03d:\n", label);
					break;
				case T_WHILE:
					// we'll need two labels
					label++;
					label++;
					// first label
					fprintf(outfile, "L%03d:\n", label - 1);
					// compile the condition
					eh_execute(node->get<Node>()->paras[0], context);
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "cmpl $0, %%eax\n");
					fprintf(outfile, "je L%03d\n", label);
					// compile the then
					eh_execute(node->get<Node>()->paras[1], context);
					// jump back to condition
					fprintf(outfile, "jmp L%03d\n", label - 1);
					fprintf(outfile, "L%03d:\n", label);
					break;
				case T_SEPARATOR:
					if(node->get<Node>()->nparas == 0)
						break;
					eh_execute(node->get<Node>()->paras[0], context);
					// discard return value
					fprintf(outfile, "popl %%eax\n");
					eh_execute(node->get<Node>()->paras[1], context);
					fprintf(outfile, "popl %%eax\n");
					break;
				case '=':
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
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
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
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
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
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
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
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
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
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
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
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
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "addl %%ecx, %%ebx\n");
					fprintf(outfile, "pushl %%ebx\n");
					break;
				case '-':
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "subl %%ecx, %%ebx\n");
					fprintf(outfile, "pushl %%ebx\n");
					break;
				case '*':
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "popl %%ebx\n");
					fprintf(outfile, "imull %%ebx\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				case '/':
					eh_execute(node->get<Node>()->paras[0], context);
					eh_execute(node->get<Node>()->paras[1], context);
					fprintf(outfile, "popl %%ecx\n");
					fprintf(outfile, "popl %%eax\n");
					fprintf(outfile, "xorl %%edx, %%edx\n");
					fprintf(outfile, "idivl %%ecx\n");
					fprintf(outfile, "pushl %%eax\n");
					break;
				default:
					printf("Unexpected opcode %d\n", node->get<Node>()->op);
					exit(0);
			}
			break;
		default:
			fprintf(stderr, "I'm too stupid a compiler; I can't handle this (type %d)\n", node->type());
			break;
	}
	return ret;
}

void EHI::eh_setarg(int argc, char **argv) {
	// how should we do this?
	return;
}

EHI::EHI() : eval_parser(nullptr) {}
EHI::~EHI() {}

ehval_p EHI::execute_cmd(const char *cmd, Array::t *paras) {
	return nullptr;
}
char *EHI::eh_getline(EHParser *parser) {
	return nullptr;
}
