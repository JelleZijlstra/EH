#!/usr/bin/ehi
include '../lib/eval.eh'

# Test the eval library

printvar exprEval "[]"
printvar universalEval "{}"
universalEval "x = 42;"
printvar x

universalEval "x + - / *"
