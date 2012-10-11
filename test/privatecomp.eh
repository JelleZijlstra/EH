#!/usr/bin/ehi
f = func:
	class A
		const private foo, bar = 1, 2
		const static baz, quux = null
	end
	printvar A
end
f()
