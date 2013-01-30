#!/usr/bin/ehi
# Test decompiling a manually constructed Node
private code = Node.T_ASSIGN(Node.T_VARIABLE("x"), 3)
echo(code.decompile())
