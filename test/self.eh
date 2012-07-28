#!/usr/bin/ehi
# What's in self?
printSelf = func: -> (printvar self)

Bool.printSelf = printSelf
true.printSelf ()

class Foo {
  new_ = func: -> (self.new ())
  initialize = printSelf
}
printvar Foo.new ()
printvar Foo.new_ ()

o = "true".length ()
