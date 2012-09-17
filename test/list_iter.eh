#!/usr/bin/ehi
# Iterate over lists
include '../lib/list.eh'
l = Cons 4, Cons 5, Nil
for i in l
	printvar i
end
