#!/usr/bin/ehi
# What's in self?
printSelf = () => (printvar this)

Bool.printSelf = printSelf
true.printSelf()

class Foo {
  new_ = () => (this.new())
  initialize = printSelf
}
printvar Foo.new()
printvar Foo.new_()

o = "true".length()
