#!/usr/bin/ehi
# Exploring oddities of EH syntax
set foo = 3
class Foo
	public foo:
		echo baz
	endfunc
endclass
echo foo
echo foo + bar
echo $foo
set foo = new Foo
set baz = $foo.'foo'
$baz:
$foo.foo:
set muh = foo
$foo.$muh:
baz:
