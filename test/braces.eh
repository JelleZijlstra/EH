#!/usr/bin/ehi
class Foo
	public bar = 3

	public baz = func: n
		ret n + this.bar
	end
end
f = func:; ret Foo.new (); end
if (4 > 3)
	echo true
else
	echo false
end
i = 3
while (i > 0); echo i; i--; end
for i in 1..5
	echo i
end
