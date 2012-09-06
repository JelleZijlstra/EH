#!/usr/bin/ehi
# Illustrate the use of explicit superclasses

class Foo
	private super = this.inherit Object
	public toString = func: 
		"the address of this object is " + super.toString()
	end
	public fooMethod = func:
		echo 'this is a Foo method'
	end
end

f = Foo.new()
echo f

class Bar
	private foo = this.inherit Foo
	public fooMethod = func:
		echo 'this is a Bar method'
		foo.fooMethod()
	end
end
b = Bar.new()
b.fooMethod()
