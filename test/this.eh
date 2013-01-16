#!/usr/bin/ehi
# Illustrate the use of this
class Foo
	private var = 1
	private bar = func: n
		echo('Private method ' + n)
		echo('Private property var is: ' + this.var)
	end
	public baz = func: n
		echo('Public method ' + n)
		this.var = 2
		this.bar n
	end
end
foo = Foo.new ()
foo.baz 42
