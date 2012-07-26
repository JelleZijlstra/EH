#!/usr/bin/ehi
# Illustrate the use of this
class Foo
	private var = 1
	private bar: n
		$echo 'Private method ' + n
		$echo 'Private property var is: ' + this.var
	endfunc
	public baz: n
		$echo 'Public method ' + n
		this.var = 2
		this.bar: n
	endfunc
endclass
foo = Foo.new:
foo.baz: 42
