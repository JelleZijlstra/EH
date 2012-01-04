#!/usr/bin/ehi
# Illustrate adding class members at runtime
class Foo
	public bar = 3
endclass
set foo = new Foo
set Foo::baz = 2
set bar = new Foo
printvar: $foo
printvar: $bar
set bar.meh = 4
printvar: $bar
