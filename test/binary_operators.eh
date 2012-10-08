#!/usr/bin/ehi
class Foo {
  operator& = func: rhs {
    echo 'Binary and with ' + rhs
  }
  operator| = func: rhs {
    echo 'Binary or with ' + rhs
  }
  operator^ = func: rhs {
    echo 'Binary xor with ' + rhs
  }
  operator~ = func: {
    echo 'Binary negation'
  }
  operator_uminus = func: {
    echo 'Unary minus'
  }
  operator! = func: {
    echo 'Negation'
  }
  # Not really
  operator* = rhs => (echo 'Unary minus')
}
f = Foo.new ()
f & 2
f ^ 3
f | 4
~f
f * -1
!f
