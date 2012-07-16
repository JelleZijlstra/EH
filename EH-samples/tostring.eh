#!/usr/bin/ehi
# Testing various toString methods
class Foo {}
o = new Foo
printvar: o.toString:
printvar: [].toString:
printvar: 2.toString:
printvar: 1.0.toString:
printvar: "string".toString:
printvar: true.toString:
printvar: null.toString:
printvar: (1..3).toString:
