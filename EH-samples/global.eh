#!/usr/bin/ehi
# More tests for the global keyword
class Foo
	public bar
	public baz: n
		put $n . ' '
		echo $myvar
	end
end
set myvar = 42
Foo::baz: 'test'
set myvar = 'foo'
Foo::baz: 'bah'
set newvar = Foo::baz
$newvar: 19
put @string 19
echo @string 19
func unknownglobal:
	printvar: $nosuchvar
end
unknownglobal:
