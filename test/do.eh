echo 1

do
    echo 2
    echo 3
end

echo 4

private f x = do
    echo x
    do
        echo(x + x)
    end
    x
end

echo (f 5)
