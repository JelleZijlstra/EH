#!/usr/bin/ehi
# Illustrate closures that are not first declared as methods
$ foo = func: n
	echo $n
endfunc
printvar: $foo
call $foo:2
