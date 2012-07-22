#!/usr/bin/ehi
foo = func: n
	a = 3
	bar: 42
	echo $a
end

bar = func: n
	a = 5
	echo $a * $n
end

foo: 5
