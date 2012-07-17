#!/usr/bin/ehi
# Illustrate type-checking functions
# true
echo is_int: 2
# false
echo is_int: false
# true
echo is_string: 'foo'
# false
echo is_string: 2
class Foo
	public bar
endclass
# true
echo is_object: Foo.new:
# false
echo is_object: 'hello'
