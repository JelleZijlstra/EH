#!/usr/bin/ehi

include '../lib/library.eh'

class Foo
	Macro.decorate(Macro.privatize, raw(toString = () => "Foo"))
end

printvar Foo
