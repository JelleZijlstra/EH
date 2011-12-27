#!/usr/bin/ehi
# Illustrate EH classes
echo 2
if 2
	echo 'meh'
endif
class Foo
	public bar
	private baz
	public meh: n
		echo $n
	endfunc
	private mah: n
		echo $n
	endfunc
endclass
echo 3
