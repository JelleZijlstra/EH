#include "eh.h"

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
				case T_WHILE:
					while(execute(node->op.paras[0]))
						execute(node->op.paras[1]);
					return 0;
				case T_SEPARATOR:
					execute(node->op.paras[0]);
					execute(node->op.paras[1]);
					return 0;
				case '=':
					return execute(node->op.paras[0]) == 
						execute(node->op.paras[1]);
				case '>':
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
