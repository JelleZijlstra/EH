#!/usr/bin/ehi
# Illustrate the use of $this
class Foo
	private var = 1
	private bar: n
		echo 'Private method ' + @string $n
		echo 'Private property var is: ' + @string $this->var
	endfunc
	public baz: n
		echo 'Public method ' + @string $n
		$ this->var = 2
		$this->bar: $n
	endfunc
endclass
set foo = new Foo
$foo->baz: 42
