#!/usr/bin/ehi
# Demonstrate the global variable, a reference to the global scope.
foo = 4
class A
	public static foo = 3
	public static f() = do
		echo foo
		echo(global.foo)
	end
end
A.f ()
echo foo
echo(global.foo)
echo(A.foo)
