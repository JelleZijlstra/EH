#!/usr/bin/ehi

class A
	public foo = 3
	class C
		public bar = func:
			echo foo
		end
	end
end

class B
	public foo = 4
	o = A.C.new ()
	o.bar ()
end
