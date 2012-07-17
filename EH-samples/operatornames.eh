#!/usr/bin/ehi
# Operator overloading!
class Foo
  public f = 3

  public operator_plus = func: rhs {
    out = new Foo
    $out.f = this.f + rhs.f
    ret out
  }
end

o1 = new Foo
$o1.f = 4
o2 = new Foo
printvar: o1 + o2
