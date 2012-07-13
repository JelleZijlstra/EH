#!/usr/bin/ehi
# This works, though arguably it shouldn't.
const foo = [1, 2]
printvar: $foo
foo->0 = 3
printvar: $foo
