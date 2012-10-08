#!/usr/bin/ehi
include '../lib/exception.eh'

# Illustrate the static and const specifiers
class Foo
	static bar
	public baz
	public mah
	const muh = 2
	const meh: n
		echo n
	endfunc

	const toString:
		"@Foo " + bar + " " + baz + " " + mah + " " + muh
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
rescue () => (Foo.muh = 42)
rescue () => (foo.muh = 42)
echo Foo.muh
