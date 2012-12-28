#!/usr/bin/ehi
# Try to break library classes; this previously segfaulted.
var = Map.new ()
funcv = var.has
printvar funcv ()
class Foo
	public bar
end
f = Foo.new ()
echo(f.bar)

printvar funcv
printvar(funcv ())
printvar(var.has())
