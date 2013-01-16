#!/usr/bin/ehi

class A
	a = 3
	class B
		public b = func:
			echo a
		end
	end
end

class C
	a = 4
	class D
		this.inherit(A.B)
	end
	o = D.new ()

	# Expect 3, because inherited methods retain their original parent scope
	o.b ()
end
