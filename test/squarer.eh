squarer () = do
    private x = 0
    while true
        x = yield(x * x)
    end
end

private sq = squarer()

# to make it reach the loop
sq.next()

echo(sq.send 3)
echo(sq.send 4)
