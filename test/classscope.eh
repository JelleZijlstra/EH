#!/usr/bin/ehi

class A
	public static foo = 3
	class C
		public bar = func:
			echo foo
		end
	end
end

class B
	public static foo = 4
	static o = A.C.new ()
	o.bar ()
end
