#!/usr/bin/ehi
# Illustrates EH references
$ foo = 3
$ bar = &foo
call printvar: $bar
echo $foo + $bar
$ foo = $foo + 2
echo $foo
echo $bar
$ bar = $bar + 2
echo $foo
echo $bar
func baz: n
	$ n = $n + 2
endfunc
call baz: &foo
echo $foo
func meh: n
	echo $n
	global foo
	echo $foo
endfunc
call meh: 1
