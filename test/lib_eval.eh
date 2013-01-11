#!/usr/bin/ehi
include '../lib/eh.eh'

# Test the eval library

printvar(EH.exprEval "[]")
printvar(EH.universalEval "{}")
private obj = EH.universalEval "x = 42;"
printvar(obj.x)

EH.universalEval "x + - / *"
