#!/usr/bin/ehi
foo = class {
	private a = 3
	public b: {
		set a++
		ret a
	}
}

o = new foo
printvar: o
printvar: o.b:
printvar: o

# We can also use new to clone objects
p = new o
printvar: p
