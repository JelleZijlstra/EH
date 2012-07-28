#!/usr/bin/ehi
class Foo
	private baz = 42
	public bar:
		printvar this
		this = 2
		echo this.baz
		ret this
	end
	public useprivate:
		printvar this
		ret baz
	end
end
f = Foo.new ()
printvar f.bar ()
printvar f
printvar f.useprivate ()
echo baz
