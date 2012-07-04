#!/usr/bin/ehi

class A {
	public foo = 3
	class C {
		public bar: {
			echo $foo
		}
	}
}

class B {
	public foo = 4
	o := new A::C
	$o->bar:
}
