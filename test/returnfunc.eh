#!/usr/bin/ehi
# Returning a function. I suspect this is currently buggy.
func a: {
	b = 3
	c = func: {
		$echo b
	}
	c:
	ret c
}
f = a:
f:
