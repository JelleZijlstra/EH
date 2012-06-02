#!/usr/bin/ehi
# Illustrates EH references
foo := 3
bar := &foo
# @int 3
printvar: $bar
# 6
echo $foo + $bar
foo := $foo + 2
# 5
echo $foo
# 5
echo $bar
bar := $bar + 2
# 7
echo $foo
# 7
echo $bar
func baz: n
	n := $n + 2
endfunc
baz: &foo
# 9
echo $foo
func meh: n
	# 1
	echo $n
	# 9
	echo $foo
endfunc
meh: 1
