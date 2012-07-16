#!/usr/bin/ehi
# Try to break library classes; this previously segfaulted.
var = new CountClass
funcv = var.docount
printvar: funcv:
class Foo
	public bar
end
f = new Foo
echo f.bar

printvar: funcv
printvar: funcv:
printvar: var.docount:
