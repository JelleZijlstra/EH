#!/usr/bin/ehi
# Test inheritance of class members

class A
	static b = 'b from A'
	static c = 'c from A'
end

class B
	this.inherit A

	echo b
end

echo(B.c)
