#!/usr/bin/ehi
include '../lib/exception.eh'

# Illustrate the static and const specifiers
class Foo
	static bar
	public baz
	public mah
	const muh = 2
	const meh n = do
		echo n
	end

	const toString() = do
		"@Foo " + bar + " " + baz + " " + mah + " " + muh
	end
end
foo = Foo()
printvar foo
Foo.bar = 2
echo(Foo.bar)
# ... And this will not work
rescue(() => (foo.bar = 3))
echo(Foo.bar)
rescue(() => (Foo.muh = 42))
# Should generate an error
rescue(() => (foo.muh = 42))
echo(Foo.muh)
