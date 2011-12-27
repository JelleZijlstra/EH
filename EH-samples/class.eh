#!/usr/bin/ehi
# Illustrate EH classes
echo 2
if 2
	echo 'meh'
endif
class Foo
	public bar = 3
	private baz
	public meh: n
		echo $n
	endfunc
	private mah: n
		echo $n
	endfunc
endclass
$ bar = new Foo
call printvar: $bar
echo $bar.bar
$ foo = $bar.meh
call printvar: $foo
