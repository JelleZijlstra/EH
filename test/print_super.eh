#!/usr/bin/ehi
# Doing funky things with superclasses

class Foo
end

class Bar
	private super = this.inherit Foo
	public printIt = func:
		printvar super
		echo super
	end
	public assignIt = func:
		super.toString = func: -> "Nana"
	end
	public cloneIt = func:
		printvar super.new()
	end
end

b = Bar.new()
b.printIt()
b.cloneIt()
b.assignIt()
