#!/usr/bin/ehi
# Illustrate the static and const specifiers
class Foo
	static bar
	public baz
	public mah
	const muh = 2
	const meh: n
		echo n
	endfunc
endclass
foo = Foo.new ()
printvar foo
# Because bar is static, this will also modify bar as it appears in foo
Foo.bar = 2
echo foo.bar
# ... And this will change bar as it appears in the class
foo.bar = 3
echo Foo.bar
# Should both generate an error
Foo.muh = 42
foo.muh = 42
echo Foo.muh
