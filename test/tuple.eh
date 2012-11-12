#!/usr/bin/ehi
t = Tuple.new 1, "true", false
printvar t
printvar t.length ()
printvar t.isA Tuple
for i in t.length()
  printvar t->i
end

# Because the EH object model isn't quite implemented, this is the only way we can actually see printvar on action on a tuple
Tuple.printvar = () => (global.printvar this)
t2 = Tuple.new false, [], 42
(t2.printvar())
