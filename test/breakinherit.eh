#!/usr/bin/ehi
class A {
	inherit A
	
	class A {
		public b
	}

	inherit A
	public a = A.new ()
}
o = A.new ()
printvar o
