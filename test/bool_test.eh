#!/usr/bin/ehi
# Illustrates the use of boolean variables in EH
foo = true
bar = false
# Math with bools
echo(2 * foo)
echo(2 * bar)
echo foo
echo bar
# Dump a bool
printvar foo
printvar bar
baz = 'test'
meh = ''
# Casting to bool
echo(baz.toBool())
echo(meh.toBool())
echo(1.toBool())
echo(0.toBool())
