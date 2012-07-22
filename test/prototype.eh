#!/usr/bin/ehi
# Illustrate access to the class prototype using the dot operator
class Foo
	public bar = 2
endclass
echo Foo.bar
mah = Foo.new:
$Foo.bar = 3
echo Foo.bar
meh = Foo.new:
echo mah.bar
echo meh.bar
