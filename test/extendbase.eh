#!/usr/bin/ehi
echo "Map"
Array##map f = do
	out = []
	for value in this
		out.push(f value)
	end
	out
end

arr = ["foo", "bar", "quux"]
f x = x.length()
printvar (arr.map f)

echo "Reduce"
Array##reduce(f, base) = do
	out = base
	for value in this
		out = f(out, value)
	end
	out
end

f(counter, value) = counter + 1
printvar (arr.reduce(f, 0))

echo "Iterate"
Array##each f = for value in this
	f value
end

arr.each echo

echo "Filter"
Array##filter f = do
	out = []
	for value in this
		if f value
			out.push value
		end
	end
	out
end

f value = value.length() > 3
printvar(arr.filter f)
