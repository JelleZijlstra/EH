#!/usr/bin/ehi
# Illustrate access to the class prototype using the dot operator
class Foo
	public bar = 2
endclass
echo Foo.bar
mah = new Foo
$Foo.bar = 3
echo Foo.bar
meh = new Foo
echo mah.bar
echo meh.bar
