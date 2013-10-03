#!/usr/bin/ehi
# Doing funky things with superclasses

class Foo
end

class Bar
	private super = this.inherit Foo
	public printIt() = do
		printvar(this.super)
		echo(this.super)
	end
	public assignIt() = do
		this.super.toString() = "Nana"
	end
	public cloneIt() = do
		printvar(this.super())
	end
end

b = Bar()
b.printIt()
b.cloneIt()
b.assignIt()
