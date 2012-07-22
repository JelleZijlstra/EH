#!/usr/bin/ehi
$Integer.operator_plus = func: rhs -> self - rhs
printvar: 1 + 1

# I came across this while implementing this test - this crashed the interpreter before I fixed the bug
$Integer.operator_plus = 42
printvar: 1 + 1
