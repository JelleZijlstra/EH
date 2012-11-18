#!/usr/bin/ehi
# Illustrate built-in object constructors

# One remaining bug in the object model is that printvar sees through the difference between pseudo-objects and real ones
printvar (Integer.new 3)
class Foo
	public toArray = () => []
end
printvar (Array.new Foo.length())
