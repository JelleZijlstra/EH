#!/usr/bin/ehi
# Illustrate type-checking functions
# true
echo(2.isA Integer)
# false
echo(false.isA Integer)
# true
echo('foo'.isA String)
# false
echo(2.isA String)
class Foo
	public bar
end
# true
echo (Foo.new().isA Object)
# false
echo('hello'.isA Object)
