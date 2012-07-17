#!/usr/bin/ehi
# The for ... as operator
arr = [ 'a' => 1, 'b' => 2, 3 => 'c' ]
echo 'Array with value only'
for $arr as value
	echo $value
end
echo 'Array with value and key'
for $arr as key => value
	echo $key
	echo $value
end
echo 'Array not previously declared'
for [1, 2, 3] as value
	echo $value
end
class Foo
	public bar = 2
	private baz = 3
	const mah = 4
end
echo 'Object'
for Foo.new: as key => value
	echo $key
	echo $value
end
