#!/usr/bin/ehi

class Foo
	public toString = func: -> "Foo instance"
end
echo (Foo.toString())
f = Foo.new()
echo f.toString()
