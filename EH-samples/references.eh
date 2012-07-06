#!/usr/bin/ehi
# Illustrates EH references
foo := 3
bar := $foo
# @int 3
printvar: $bar
# 6
echo $foo + $bar
foo := $foo + 2
# 5
echo $foo
# 3
echo $bar
bar := $bar + 2
# 5
echo $foo
# 5
echo $bar
func baz: n
	n := $n + 2
end
baz: $foo
# 5
echo $foo
func meh: n
	# 1
	echo $n
	# 5
	echo $foo
end
meh: 1
