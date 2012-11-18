#!/usr/bin/ehi
printvar(3.isA Integer)
printvar(4.isA Bool)
class Foo
end
f = Foo.new()
printvar(f.isA Foo)
printvar(Foo.new().isA f)
