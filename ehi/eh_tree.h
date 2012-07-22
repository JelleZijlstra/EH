void free_node(ehretval_p in);
opnode_t *eh_addnode(int opcode);
opnode_t *eh_addnode(int opcode, ehretval_p first);
opnode_t *eh_addnode(int opcode, ehretval_p first, ehretval_p second);
opnode_t *eh_addnode(int opcode, ehretval_p first, ehretval_p second, ehretval_p third);
opnode_t *eh_addnode(int opcode, ehretval_p first, ehretval_p second, ehretval_p third, ehretval_p fourth);
void print_tree(const ehretval_p in, const int n);
