#!/usr/bin/ehi
t = Tuple.new(1, "true", false)
printvar t
printvar(t.length ())
printvar(t.isA Tuple)
for i in t.length()
  printvar(t->i)
end

Tuple##printvar() = (global.printvar this)
t2 = Tuple(false, [], 42)
t2.printvar()
