#!/usr/bin/ehi
Integer.operator+ = func: rhs -> (this - rhs)
printvar 1 + 1

# I came across this while implementing this test - this crashed the interpreter before I fixed the bug
Integer.operator+ = 42
printvar 1 + 1
