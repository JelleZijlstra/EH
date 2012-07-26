#!/usr/bin/ehi
$echo 2
func foo:
	ret 1
endfunc
bar = 2
foo: &bar
