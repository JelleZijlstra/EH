#!/usr/bin/ehi
# Explore function arguments and expressions
func giveargs: a, b, c
	echo a
	echo b
	echo c
	ret 0
end
# Expect 1
printvar(giveargs(1, 2, 3) + 1)
# Expect 0
printvar giveargs(1, 2, 3 + 1)
