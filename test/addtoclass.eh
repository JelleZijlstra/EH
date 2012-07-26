#!/usr/bin/ehi
# Illustrate adding class members at runtime
class Foo
	public bar = 3
endclass
foo = Foo.new: ()
Foo.baz = 2
bar = Foo.new: ()
printvar: foo
printvar: bar
bar.meh = 4
printvar: bar
