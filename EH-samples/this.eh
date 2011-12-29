#!/usr/bin/ehi
# Illustrate the use of $this
class Foo
	private bar: n
		echo 'Private method ' + @string $n
	endfunc
	public baz: n
		echo 'Public method ' + @string $n
		$this.bar: $n
	endfunc
endclass
set foo = new Foo
$foo.baz: 42
