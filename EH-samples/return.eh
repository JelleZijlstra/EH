#!/usr/bin/ehi
func test: n
	echo foo: $n
	printvar: foo: $n
	ret (foo: $n)
endfunc
func foo: n
	echo $n
	ret $n
endfunc
echo (test: 2)
printvar: test: 2

