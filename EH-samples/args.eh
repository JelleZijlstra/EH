#!/usr/bin/ehi
func test: a, b
	echo 'First argument: ' + @string $a
	echo 'Second argument: ' + @string $b
endfunc
call test: 1, 2
