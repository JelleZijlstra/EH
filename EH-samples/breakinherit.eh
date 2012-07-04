#!/usr/bin/ehi
class A {
	inherit A
	
	class A {
		public b
	}

	inherit A
	public a = new A
}
o := new A
printvar: $o
