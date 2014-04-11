public range start stop step = do
    private i = start
    while true
        yield i
        i += step
        # this instead of a condition on the loop to test the break statement
        if i >= stop
            break
        end
    end
end

for elt in range 1 5 2
    echo elt
end
