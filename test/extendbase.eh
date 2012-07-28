#!/usr/bin/ehi
echo "Map"
Array.map = func: f {
	out = []
	for self as key => value {
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
	for self as value {
		out = f out, value
	}
	ret out
}

f = func: counter, value -> counter + 1
printvar (arr.reduce f, 0)

echo "Iterate"
Array.each = func: f {
	for self as key => value {
		f key, value
	}
}

arr.each (func: key, value { echo value; })

echo "Filter"
Array.filter = func: f {
	out = []
	for self as key => value {
		if (f key, value) {
			out->key = value
		}
	}
	ret out
}

f = func: key, value -> (value.length ()) > 3
printvar arr.filter f
