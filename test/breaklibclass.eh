#!/usr/bin/ehi
# Try to break library classes; this previously segfaulted.
var = CountClass.new:
funcv = var.docount
printvar: funcv:
class Foo
	public bar
end
f = Foo.new:
$echo f.bar

printvar: funcv
printvar: funcv:
printvar: var.docount:
