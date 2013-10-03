#!/usr/bin/ehi
# Illustrate the use of explicit superclasses

class Foo
	this.inherit Object
	public toString() = do
		"the address of this object is " + Object##toString.bindTo(this)()
	end
	public fooMethod() = do
		echo 'this is a Foo method'
	end
end

f = Foo()
echo f

class Bar
	this.inherit Foo
	public fooMethod() = do
		echo 'this is a Bar method'
		Foo##fooMethod.bindTo(this)()
	end
end
b = Bar()
b.fooMethod()
