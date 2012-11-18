#!/usr/bin/ehi
# What if something throws in the uncaught exception handler
class Thrower
	public toString = () => throw 42
end
throw(Thrower.new())
