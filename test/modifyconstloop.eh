#!/usr/bin/ehi
class Foo
	const static a = 3
end
printvar Foo
for i in 1
	Foo.a = 4
end
printvar Foo
