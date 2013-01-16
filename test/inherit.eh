#!/usr/bin/ehi
class A
	public a = 3
	public b = func:
		echo 42
	end
end
class B
	this.inherit A
	public c = 5
	public d = func:
		echo 4
	end
end
o = B.new ()
o.b ()
