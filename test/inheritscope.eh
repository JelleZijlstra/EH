#!/usr/bin/ehi

class A {
	a = 3
	class B {
		public b: {
			$echo a
		}
	}
}

class C {
	a = 4
	class D {
		inherit: A.B
	}
	o = D.new: ()
	
	# Expect 3, because inherited methods retain their original parent scope
	o.b: ()
}
