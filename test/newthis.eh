#!/usr/bin/ehi
class Foo
	private baz = 42
	public bar = func:
		printvar this
		ret this
	end
	public useprivate = func:
		ret this.baz
	end
end
f = Foo.new ()
printvar(f.bar())
printvar(f.useprivate())
echo baz
