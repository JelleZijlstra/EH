#!/usr/bin/ehi
# The "type" method
printvar(1.type())
printvar([].type())
a = "true"
printvar(a.type())
class Foo; end
f = Foo.new()
printvar(f.type())
