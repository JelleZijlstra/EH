#!/usr/bin/ehi
echo "Map"
Array.map = func: f
	out = []
	for key, value in this
		out->key = f value
	end
	out
end

arr = ["foo", "bar", "quux"]
f = x => (x.length ())
printvar (arr.map f)

echo "Reduce"
Array.reduce = func: f, base
	out = base
	for _, value in this
		out = f(out, value)
	end
	out
end

f = counter, value => counter + 1
printvar (arr.reduce(f, 0))

echo "Iterate"
Array.each = func: f
	for key, value in this
		f(key, value)
	end
end

arr.each (key, value => echo value)

echo "Filter"
Array.filter = func: f
	out = []
	for key, value in this
		if f(key, value)
			out->key = value
		end
	end
	out
end

f = key, value => value.length() > 3
printvar(arr.filter f)
