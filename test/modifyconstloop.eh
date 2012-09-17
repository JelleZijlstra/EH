#!/usr/bin/ehi
class Foo {
	const a = 3
}
printvar Foo
for i in 1 {
	Foo.a = 4
}
printvar Foo
