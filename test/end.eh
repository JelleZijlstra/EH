#!/usr/bin/ehi
# Illustrate the "end" keyword
class Foo
	public bar = func: n
		echo n
	end
end
func test: n
	echo n
end
baz = func: n
	echo n
end
n = given 2
	case 1
		41
	case 2
		42
end
while n
	n--
end
for i in 2
	echo i
end
if 4 > 2
	echo 3
else
	echo 1
end
mah = Foo.new ()
mah.bar 42
test 42
baz 42
