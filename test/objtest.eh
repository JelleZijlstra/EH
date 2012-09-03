#!/usr/bin/ehi
# More tests for objects
class Foo
	public bar = 0
	public set1:
		this.bar = 1
	end
	public set2:
		this.bar = 2
	end
	public useprop:
		# Will use object property
		echo this.bar
	end
end
o = Foo.new ()

# 0
echo o.bar
(Foo.set1 ())
# 1
echo Foo.bar
# 0
echo o.bar
o.set2 ()
# 1
echo Foo.bar
# 2
echo o.bar
# 2
o.useprop ()
