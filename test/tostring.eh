#!/usr/bin/ehi
# Testing various toString methods
class Foo {}
o = Foo.new:
# This uses the standard Object.toString method, which prints the object's memory address and is therefore inherently unportable and generally unpredictable. On my system at least, its return value is consistent from one invocation of the interpreter to another.
printvar: o.toString:
printvar: [].toString:
printvar: 2.toString:
printvar: 1.0.toString:
printvar: "string".toString:
printvar: true.toString:
printvar: null.toString:
printvar: (1..3).toString:
