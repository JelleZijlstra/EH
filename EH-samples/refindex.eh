#!/usr/bin/ehi
# This is totally crazy, but at least it shouldn't crash ehi.
# (It is significantly less crazy now that we no longer support references.)
foo = 3
bar = foo
bar->foo = 1
printvar: $bar
printvar: $foo
echo $bar->foo
echo $foo->$bar
