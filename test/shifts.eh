#!/usr/bin/ehi
# The shift operators

class Foo
	operator<< = x => (echo 'Left shift' + x)
	operator>> = x => (echo 'Right shift' + x)
end

f = Foo.new()
f << 3
f >> 3
f <<= 3
printvar f
