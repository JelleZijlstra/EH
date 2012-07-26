#!/usr/bin/ehi
# Constructors can now take arguments
class Foo {
  initialize = func: a, b {
    $echo a
    $echo b
  }
}
o1 = Foo.new: "a", "b"
o2 = Foo.new:
