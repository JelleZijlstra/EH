#!/usr/bin/ehi
# Illustrate access to the class prototype using the dot operator
class Foo
	public bar = 2
end
echo(Foo##bar)
mah = Foo()
Foo##bar = 3
echo(Foo##bar)
meh = Foo()
echo(mah.bar)
echo(meh.bar)
