#!/usr/bin/ehi
class Foo {
  operator_and = func: rhs {
    echo 'Binary and with ' + rhs
  }
  operator_or = func: rhs {
    echo 'Binary or with ' + rhs
  }
  operator_xor = func: rhs {
    echo 'Binary xor with ' + rhs
  }
  operator_tilde = func: {
    echo 'Binary negation'
  }
  operator_uminus = func: {
    echo 'Unary minus'
  }
  operator_bang = func: {
    echo 'Negation'
  }
}
f = Foo.new:
$f & 2
$f ^ 3
$f | 4
~f
-f
!f
