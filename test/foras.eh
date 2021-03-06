#!/usr/bin/ehi
# The for ... as operator
arr = { 'a': 1, 'b': 2, 'c': 'c'}
echo 'Hash with value only'
for _, value in arr
	echo value
end
echo 'Array with value and key'
for (key, value) in arr
	echo key
	echo value
end
echo 'Array not previously declared'
for value in [1, 2, 3]
	echo value
end
class Foo
	public bar = 2
	private baz = 3
	const mah = 4
end
echo 'Object'

for key in Foo.new().members()
	echo key
end
