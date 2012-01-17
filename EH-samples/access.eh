#!/usr/bin/ehi
# Try to find array access bug
set foo = 1
for (count $foo) count i
	echo $foo->$i
end
echo $foo->1
