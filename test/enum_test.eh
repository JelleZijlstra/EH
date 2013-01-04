#!/usr/bin/ehi

echo 'Testing echo, then printvar'

enum List
	Nil, Cons(head, tail)
end

echo 'Enum class'
echo List
printvar List

echo 'Enum nullary member'
echo(List.Nil)
printvar(List.Nil)

echo 'Enum member with arguments'
echo(List.Cons)
printvar(List.Cons)

echo 'Enum instance'
e = List.Cons(3, List.Nil)
echo e
printvar e

echo 'Enum instance members'
echo(e->0)
printvar(e->1)
