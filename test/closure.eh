#!/usr/bin/ehi
# Illustrate that closures can exist in ehi
class Foo
	public bar: n
		$echo n * n
		ret 42
	endfunc
endclass
foo = Foo.new:
printvar: foo
baz = foo.bar
printvar: baz
baz: 2
printvar: baz
