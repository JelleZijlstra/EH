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
		# Illustrate private method access from class context
		$ bar = new Foo
		call $bar . mah : 'Calling a private method'
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
call $bar . meh : 42
# Error: method is private
call $bar . mah : 'Calling a private method'
