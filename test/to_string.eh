#!/usr/bin/ehi

class Foo
	public toString = () => "Foo instance"
end
echo (Foo.toString())
f = Foo.new()
echo (f.toString())
