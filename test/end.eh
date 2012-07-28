#!/usr/bin/ehi
# Illustrate the "end" keyword
class Foo
	public bar: n
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
	set n--
end
for 2 count i
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
