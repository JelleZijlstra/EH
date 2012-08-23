#!/usr/bin/ehi
# The shift operators

class Foo
	operator<< = func: x -> (echo 'Left shift' + x)
	operator>> = func: x -> (echo 'Right shift' + x)
end

f = Foo.new()
f << 3
f >> 3
f <<= 3
printvar f
