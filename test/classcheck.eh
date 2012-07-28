#!/usr/bin/ehi
# Illustrate the isA method, as well as parameter checking for library functions.
class Foo
	public bar = 42
end
bar = Foo.new ()
# true
echo bar.isA Foo
# error
echo bar.isA ()
# error
echo pow 3
# error
echo pow (3, 2, false)
# false
class Bar{}
echo bar.isA Bar
