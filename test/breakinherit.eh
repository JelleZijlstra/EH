#!/usr/bin/ehi
class A {
	try {
		inherit A
	} catch {
		echo 'Too bad, that did not work'
	}
	
	class A {
		public b
	}

	inherit A
	public a = A.new ()
}
o = A.new ()
printvar o
