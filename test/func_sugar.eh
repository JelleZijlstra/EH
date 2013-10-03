#!/usr/bin/ehi

class Foo
	static public foo x = x * x

	static public bar z = z + z

	static public baz(a, b) = a + b
end

printvar Foo
printvar(Foo.foo 3)
printvar(Foo.bar 2)
printvar(Foo.baz(4, 5))
