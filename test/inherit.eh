#!/usr/bin/ehi
class A
	public a = 3
	public b:
		echo 42
	end
end
class B
	this.inherit A
	public c = 5
	public d:
		echo 4
	end
end
o = B.new ()
o.b ()
