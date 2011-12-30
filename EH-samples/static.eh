#!/usr/bin/ehi
# Illustrate the static and const specifiers
class Foo
	static bar
	public baz
	mah
	const meh: n
		echo $n
	endfunc
endclass
$ foo = new Foo
printvar: $foo
# Because bar is static, this will also modify bar as it appears in $foo
$ Foo::bar = 2
echo $foo.bar
# ... And this will change bar as it appears in the class
$ foo.bar = 3
echo $Foo::bar
