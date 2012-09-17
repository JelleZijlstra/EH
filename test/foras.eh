#!/usr/bin/ehi
# The for ... as operator
arr = [ 'a' => 1, 'b' => 2, 3 => 'c' ]
echo 'Array with value only'
for ((), value) in arr
	echo value
end
echo 'Array with value and key'
for (key, value) in arr
	echo key
	echo value
end
echo 'Array not previously declared'
for ((), value) in [1, 2, 3]
	echo value
end
class Foo
	public bar = 2
	private baz = 3
	const mah = 4
end
echo 'Object'
# This now doesn't print anything, because all the properties are in the parent
# object. Not sure that should be considered desired behavior.
for ((), key) in ((Foo.new()).members())
	echo key
end
