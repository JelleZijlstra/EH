#!/usr/bin/ehi
include '../lib/assert.eh'

enum List
	Nil, Cons(head, tail)
end

enum X
	A, B
end

assert(List != X)
b = List
assert(b == List)
assert(List.Nil != List.Cons)
assert(List.Nil != X.A)
c = List.Nil
d = List.Nil
assert(c == d)

e = List.Cons(3, List.Nil)
f = List.Cons(4, List.Nil)
g = List.Cons(3, List.Nil)
assert(e != f)
assert(e == g)
