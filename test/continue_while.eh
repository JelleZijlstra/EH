private i = 0

while true
    i += 1
    echo i

    if i < 2
        continue
    end

    if i > 5
        break
    end

    echo "got to the end!"
end

for i in 6
    echo i

    if i < 2
        continue
    end

    if i > 5
        break
    end

    echo "got to the end!"
end
