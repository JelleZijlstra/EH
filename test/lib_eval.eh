#!/usr/bin/ehi
include '../lib/eval.eh'

# Test the eval library

printvar(exprEval "[]")
printvar(universalEval "{}")
private obj = universalEval "x = 42;"
printvar(obj.x)

universalEval "x + - / *"
