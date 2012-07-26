#!/usr/bin/ehi
# Can printvar handle this?
class B {}
class A {
	public foo = B
}
B.o = A.new:
printvar: B
