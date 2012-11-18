#!/usr/bin/ehi
# This should now also be possible
f = func:
	foo = []
	for i in 5
		foo->i = i
	end
	foo
end
printvar(f())
