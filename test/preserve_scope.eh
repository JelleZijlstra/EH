#!/usr/bin/ehi
# It's almost like JavaScript
factory = func: n
	private foo = n
	public bar = () => foo
	scope
end
o = factory 42
printvar o
printvar(o.bar())
printvar(o.foo)
