#!/usr/bin/ehi
class Foo {
	public bar = 3

	public baz: n {
		ret $n + $this->bar
	}
}
f = func: { ret new Foo; }
if 4 > 3 {
	echo true
} else {
	echo false
}
i = 3
while $i > 0 { echo $i; set i--; }
for 1..5 count i {
	echo $i
}
