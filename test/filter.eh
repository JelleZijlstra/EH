enum Listx
    Nil, Cons(head, tail)
end

public make_list arg = do
    private inner_fn it = try
        private elt = it.next()
        Listx.Cons(elt, inner_fn it)
    catch if exception.isA EmptyIterator
        Listx.Nil
    end
    return inner_fn (arg.getIterator())
end


public filter f it = for elt in it
    if f elt
        yield elt
    end
end

echo(make_list (filter (x => x % 2 == 0) [1, 2, 3, 4]))
