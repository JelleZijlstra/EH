#!/usr/bin/ehi
# Try to find array access bug
foo = 1
for i in (foo.length()) * 8
	echo (foo.getBit i)
end
echo (foo.getBit 1)
