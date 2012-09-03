#!/usr/bin/ehi
# What's in self?
printSelf = func: -> (printvar this)

Bool.printSelf = printSelf
true.printSelf()

class Foo {
  new_ = func: -> (this.new())
  initialize = printSelf
}
printvar Foo.new()
printvar Foo.new_()

o = "true".length()
