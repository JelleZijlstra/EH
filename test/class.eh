#!/usr/bin/ehi
include '../lib/exception.eh'

# Illustrate EH classes
echo 2
if 2
	echo 'meh'
endif
class Foo
	public bar = 3
	private baz
	public meh: n
		echo n
		# Illustrate private method access from class context
		this.bar = Foo.new ()
		this.bar.mah 'Calling a private method'
	endfunc
	private mah: n
		echo n
	endfunc
	public toString:
		'@Foo (' + this.bar + ', ' + this.baz + ')'
	endfunc
endclass
bar = Foo.new ()
printvar bar
echo bar.bar
foo = bar.meh
printvar foo
bar.meh 42
# Error: method is private
rescue () => (bar.mah 'Calling a private method')
bar.bar = 2
echo bar.bar
