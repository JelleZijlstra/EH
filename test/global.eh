#!/usr/bin/ehi
# More tests for the global keyword
class Foo
	public bar
	public static baz n = do
		put(n.toString() + ' ')
		echo myvar
	end
end
myvar = 42
Foo.baz 'test'
myvar = 'foo'
Foo.baz 'bah'
newvar = Foo.baz
newvar 19
put(19.toString())
echo(19.toString())
unknownglobal() = do
	printvar nosuchvar
end
unknownglobal ()
