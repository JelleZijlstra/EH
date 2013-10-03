#!/usr/bin/ehi

class A
	static a = 3
	class B
		public b() = do
			echo a
		end
	end
end

class C
	static a = 4
	class D
		this.inherit(A.B)
	end
	static o = D()

	# Expect 3, because inherited methods retain their original parent scope
	o.b()
end
