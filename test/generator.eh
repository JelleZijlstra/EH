
private range = n => do
    echo 'starting generator'
    i = 0
    while i < n
        yield i
        echo('yielded ' + i.toString() + ' in generator')
        i++
    end
end

echo(range 2)
x = range 2
printvar x
echo(x.type())

for i in range 2
    echo i
end

echo 'map'

private map = f => it => do
    for i in it
        yield(f i)
    end
end

for i in map (x => x * x) (range 3)
    echo i
end
