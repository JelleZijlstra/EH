#!/usr/bin/ehi
class A {
	public a = 3
	public b: {
		echo 42
	}
}
class B {
	inherit A
	public c = 5
	public d: {
		echo 4
	}
}
o := new B
$o->b:
