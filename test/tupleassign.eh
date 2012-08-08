#!/usr/bin/ehi
include '../lib/tuple.eh'
include '../lib/array.eh'
include '../lib/list.eh'

a, b = 1, 2
printvar a
printvar b

a, b = [3, 4]
printvar a
printvar b

a, b = Cons 5, Nil
printvar a
echo b

# This doesn't work now if you do a, (b, c) because the parentheses are essentially ignored. Fixing this will require some yacc hacking.
(a, b), c = [6, 7], 8
printvar a
printvar b
printvar c

# It even works with strings
a, b, c, d = 'test'
printvar a
printvar b
printvar c
printvar d
