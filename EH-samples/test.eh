#!/usr/bin/ehi
# EH input file illustrating various language features
// This is a comment
# This is another
echo 2
func var: a, b
	echo 'This is function var'
	echo $a
	echo $b
	ret $a + $b
endfunc
$ bar = 3
echo $bar
echo 'test'
if 2 = 2
	echo 2
	$ bar = var: 1, 3
	echo $bar
else
	echo 'This is false'
endif
call var: 2, 3
echo 'bar'
$ bar = var: 1, 5
if 2 = 2
	echo 'hi'
	echo 'a'
endif
if 2 = 3
	echo 'b'
endif
echo $bar
while $bar
	$ bar = $bar - 1
	echo $bar
endwhile