#!/usr/bin/ehi
# Operator overloading!
class Foo
  public f = 3

  public operator+ = func: rhs {
    out = Foo.new ()
    out.f = this.f + rhs.f
    ret out
  }
end

o1 = Foo.new ()
o1.f = 4
o2 = Foo.new ()
printvar o1 + o2
