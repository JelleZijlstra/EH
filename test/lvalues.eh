#!/usr/bin/ehi
# Fun with lvalues

a = b = 3
printvar a
printvar b

# Error
(() => null) = 42
