#!/usr/bin/ehi
# Illustrate closures that are not first declared as methods
foo = func: n
	$echo n
end
printvar: foo
foo:2
