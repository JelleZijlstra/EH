#!/usr/bin/ehi
# Iterate over lists
include '../lib/library.eh'
l = Cons(4, Cons(5, Nil))
for i in l
	printvar i
end
