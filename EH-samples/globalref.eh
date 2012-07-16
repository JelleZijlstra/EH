#!/usr/bin/ehi
# Demonstrate the $global variable, a reference to the global scope.
foo = 4
class A {
	public foo = 3
	public f: {
		echo foo
		echo global.foo
	}
}
$A.f:
echo foo
echo global.foo
echo A.foo
