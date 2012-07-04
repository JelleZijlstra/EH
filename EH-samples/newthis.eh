#!/usr/bin/ehi
class Foo
	private baz = 42
	public bar:
		printvar: $this
		ret $this
	end
	public useprivate:
		ret $baz
	end
end
f := new Foo
printvar: $f->bar:
printvar: $f->useprivate:
echo $baz
