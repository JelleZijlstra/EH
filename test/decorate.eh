#!/usr/bin/ehi

include '../lib/library.eh'

class Foo
	Macro.decorate(Macro.privatize, raw(bar = () => "Foo"))
end

rescue(() => echo(Foo().bar))
