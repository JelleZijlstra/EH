map f it = for elt in it
    yield(f elt)
end

gen = map String [3, 4, 5]
printvar gen

for number in map String [3, 4, 5]
    printvar number
end
