#!/usr/bin/ehi
class A {
	try {
		this.inherit A
	} catch {
		echo 'Too bad, that did not work'
	}
	
	class A {
		public b
	}

	this.inherit A
	public a = A.new()
}
o = A.new()
printvar o
printvar A
