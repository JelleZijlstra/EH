#!/usr/bin/ehi
class Foo {
	operator_colon = func: n {
		echo n
	}
}
(Foo.new:): 42
