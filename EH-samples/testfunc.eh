#!/usr/bin/ehi
# Illustrate functions
func test: n
	echo $n
	echo 'hi'
	ret $n
endfunc

call test: 1

call test: 2

$ b = test: 3

echo $b
