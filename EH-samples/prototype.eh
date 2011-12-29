#!/usr/bin/ehi
# Illustrate access to the class prototype using the double colon operator
class Foo
	public bar = 2
endclass
echo $Foo::bar
$ mah = new Foo
set Foo::bar = 3
echo $Foo::bar
$ meh = new Foo
echo $mah.bar
echo $meh.bar
