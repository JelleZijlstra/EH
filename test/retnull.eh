#!/usr/bin/ehi
f = func: x
	if x % 2 == 0
		ret
	end
	echo "it's odd"
	x
end
printvar f 2
printvar f 3
