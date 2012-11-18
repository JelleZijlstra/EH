#!/usr/bin/ehi
class Foo
	a = func:
		this.d = 42
		b = Foo.new()
		b.c()
		this.c()
	end
	
	c = () => printvar this
end
f = Foo.new()
f.a()

class Bar
	static i = 2
	private bar = null
	initialize = func: arg
		if arg != null
			this.bar = Bar.new()
		end
	end
	
	recurse = func:
		printvar this
		if this.bar != null
			this.bar.recurse()
		end
	end
end

b = Bar.new 2
b.recurse()
