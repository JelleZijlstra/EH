enum List
    Nil, Cons(head, tail)
end

echo List

private lst = List.Cons(3, List.Nil)

echo lst

echo(lst.constructor())
