#!/usr/bin/ehi
include '../lib/exception.eh'

class Foo
	private baz = 42
	public bar = func:
		printvar this
		this = 2
		echo this.baz
		this
	end
	public useprivate = func:
		printvar this
		this.baz
	end
end
f = Foo.new ()
rescue(() => printvar (f.bar ()))
printvar f
printvar(f.useprivate())
echo baz
