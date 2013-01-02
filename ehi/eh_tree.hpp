void free_node(ehval_p in);
Node::t *eh_addnode(int opcode);
Node::t *eh_addnode(int opcode, ehval_p first);
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second);
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third);
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third, ehval_p fourth);
void print_tree(const ehval_p in, const int n);
