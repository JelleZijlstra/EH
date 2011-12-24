#include "eh.h"
// symbol table for variables
#define VARTABLE_S 1024
ehvar_t *vartable[VARTABLE_S];

bool insert_variable(ehvar_t *var);
ehvar_t *get_variable(char *name);
bool has_variable(char *name );
static unsigned int hash(char *data);

int execute(ehnode_t *node) {
	// variable used
	ehvar_t *var;

	if(node == NULL)
		return 0;
	//printf("Executing nodetype %d\n", node->type);
	switch(node->type) {
		case idnode_enum:
			return (int) node->id.name;
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
				case T_SET:
					var = get_variable(node->op.paras[0]->id.name);
					if(var == NULL) {
						var = Malloc(sizeof(ehnode_t));
						var->name = node->op.paras[0]->id.name;
						// only supporting integer variables at present
						var->type = int_enum;
						insert_variable(var);
					}
					var->intval = execute(node->op.paras[1]);
					return 0;
				case '$': // variable dereference
					var = get_variable(node->op.paras[0]->id.name);
					return var->intval;
				default:
					printf("Unexpected opcode %d\n", node->op.op);
					exit(0);
			}
	}
	return 0;
}

bool insert_variable(ehvar_t *var) {
	unsigned int vhash;
	
	vhash = hash(var->name);
	if(vartable[vhash] == NULL) {
		vartable[vhash] = var;
		var->next = NULL;
	}
	else {
		var->next = vartable[vhash];
		vartable[vhash] = var;
	}
	return true;
}

ehvar_t *get_variable(char *name) {
	unsigned int vhash;
	ehvar_t *currvar;
	
	vhash = hash(name);
	currvar = vartable[vhash];
	while(currvar != NULL) {
		if(strcmp(currvar->name, name) == 0) {
			return currvar;
		}
		currvar = currvar->next;
	}
	return NULL;
}
bool has_variable(char *name ) {
	unsigned int vhash;
	ehvar_t *currvar;
	
	vhash = hash(name);
	currvar = vartable[vhash];
	while(currvar != NULL) {
		if(strcmp(currvar->name, name) == 0) {
			return true;
		}
		currvar = currvar->next;
	}
	return false;
}

/* Hash function */
// from http://azillionmonkeys.com/qed/hash.html
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

static unsigned int hash(char *data) {
    unsigned int hash, tmp, len;
    int rem;
    
    len = strlen(data);
    hash = len;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= data[sizeof (uint16_t)] << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += *data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash % VARTABLE_S;
}
