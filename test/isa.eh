#!/usr/bin/ehi
printvar(3.isA Integer)
printvar(4.isA Bool)
class Foo
end
f = Foo()
printvar(f.isA Foo)
printvar(Foo().isA Foo)
