#!/usr/bin/ehi
# Illustrate adding class members at runtime
class Foo
	public bar = 3
endclass
foo = new Foo
Foo->baz = 2
bar = new Foo
printvar: $foo
printvar: $bar
bar->meh = 4
printvar: $bar
