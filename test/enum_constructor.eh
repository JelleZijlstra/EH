include '../lib/library.eh'

echo List

lst = Cons(3, Nil)
echo lst
printvar(List.Cons)
echo(lst.type())

printvar(lst.constructor())
