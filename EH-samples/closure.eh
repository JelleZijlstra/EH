#!/usr/bin/ehi
# Illustrate that closures can exist in ehi
class Foo
	public bar: n
		echo $n * $n
		ret 42
	endfunc
endclass
$ foo = new Foo
call printvar: $foo
$ baz = $foo->bar
call printvar: $baz
call $baz: 2
printvar: $baz
