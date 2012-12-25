#!/usr/bin/ehi

enum List
	Nil, Cons(head, tail)
end

e = List.Cons(3, List.Nil)
printvar e
echo(e.head)
printvar(e.tail)
