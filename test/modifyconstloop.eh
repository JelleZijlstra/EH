#!/usr/bin/ehi
class Foo {
	const a = 3
}
printvar: Foo
for Foo as key => value {
	value = 4
}
printvar: Foo
