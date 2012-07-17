#!/usr/bin/ehi
# Can printvar handle this?
class A {
	public foo = $global
}
o = A.new:
printvar: $global
