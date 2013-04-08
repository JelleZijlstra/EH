#!/usr/bin/ehi
echo "Map"
Array.map = func: f
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
Array.reduce = func: f, base
	out = base
	for value in this
		out = f(out, value)
	end
	out
end

f(counter, value) = counter + 1
printvar (arr.reduce(f, 0))

echo "Iterate"
Array.each = func: f
	for value in this
		f value
	end
end

arr.each(value => echo value)

echo "Filter"
Array.filter = func: f
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
