#!/usr/bin/ehi
foo = class
	private a = 3
	public b = () => this.a++
end

o = foo.new()
printvar o
printvar(o.b())
printvar o

p = o.clone()
printvar p
