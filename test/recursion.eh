#!/usr/bin/ehi
func test: n
	$echo n
	if (n > 10)
		ret n
	else
		ret (test: (test: (n + 1)))
	endif
endfunc
$echo test: 0
