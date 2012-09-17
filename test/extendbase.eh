#!/usr/bin/ehi
echo "Map"
Array.map = func: f {
	out = []
	for (key, value) in this {
		out->key = f value
	}
	ret out
}

arr = ["foo", "bar", "quux"]
f = func: x -> (x.length ())
printvar (arr.map f)

echo "Reduce"
Array.reduce = func: f, base {
	out = base
	for ((), value) in this {
		out = f out, value
	}
	ret out
}

f = func: counter, value -> counter + 1
printvar (arr.reduce f, 0)

echo "Iterate"
Array.each = func: f {
	for (key, value) in this {
		f key, value
	}
}

arr.each (func: key, value { echo value; })

echo "Filter"
Array.filter = func: f {
	out = []
	for (key, value) in this {
		if (f key, value) {
			out->key = value
		}
	}
	ret out
}

f = func: key, value -> (value.length ()) > 3
printvar arr.filter f
