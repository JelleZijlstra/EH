#!/usr/bin/ehi
func test: a, b
	echo 'First argument: ' . $a
	echo 'Second argument: ' . $b
endfunc
call test: 1, 2
