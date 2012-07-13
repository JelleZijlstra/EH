#!/usr/bin/ehi
# Illustrate the class_is function, as well as parameter checking for library functions.
class Foo
	public bar = 42
end
bar = new Foo
# true
echo class_is: Foo, $bar
# error
echo class_is:
# error
echo class_is: Foo
# error
echo class_is: Foo, $bar, false
# false
echo class_is: Bar, $bar
