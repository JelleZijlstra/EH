#!/usr/bin/ehi
# Illustrates the use of boolean variables in EH
$ foo = true
$ bar = false
# Math with bools
echo 2 * $foo
echo 2 * $bar
echo $foo
echo $bar
# Dump a bool
call printvar: $foo
call printvar: $bar
$ baz = 'test'
$ meh = ''
# Casting to bool
echo @bool $baz
echo @bool $meh
echo @bool 1
echo @bool 0
