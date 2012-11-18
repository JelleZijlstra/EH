#!/usr/bin/ehi
include '../lib/library.eh'

a, b = 1, 2
printvar a
printvar b

a, b = [3, 4]
printvar a
printvar b

a, b = Cons(5, Nil)
printvar a
echo b

# Trickery with parentheses
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
