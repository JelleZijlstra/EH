#!/usr/bin/ehi
class Foo
	public bar = 3
endclass
$ baz = new Foo
#call $baz.bar
echo $baz.bar
