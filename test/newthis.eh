#!/usr/bin/ehi
class Foo
	private baz = 42
	public bar:
		printvar this
		ret this
	end
	public useprivate:
		ret this.baz
	end
end
f = Foo.new ()
printvar f.bar ()
printvar f.useprivate ()
echo baz
