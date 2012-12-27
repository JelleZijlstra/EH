#!/usr/bin/ehi

class A
	f = func:
		private x = 3
		() => x
	end
end
private inner_f = A.f()
echo(inner_f())
