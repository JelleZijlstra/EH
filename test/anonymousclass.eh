#!/usr/bin/ehi
foo = class
	private a = 3
	public b = () => this.a++
end

o = foo.new()
printvar o
printvar(o.b())
printvar o

# We can also use to.new clone objects
p = o.new()
printvar p
