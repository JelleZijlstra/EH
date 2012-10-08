#!/usr/bin/ehi
include '../lib/exception.eh'

# Test whether you can modify const properties.
class Foo
	private const baz = 42
	private const quuz = 42
	public foo:
		baz = 43
		this.quuz = 43
	end
end
f = Foo.new ()
printvar f
rescue () => (f.foo())
printvar f
