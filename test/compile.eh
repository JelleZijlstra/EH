#!/usr/bin/ehi
# Example of an EH input file accepted by the compiler (which does not
# currently accept variables, function calls, or for loops, and even for things
# that it does accept generates spectacularly inefficient assembly code).
#  a = 3
echo 42
if 2 == 2
	3 + 3
	echo (2 - 3)
end
echo 43
if 3 - 3
	echo 2
end
echo 44
if (3 > 2)
	echo 1
end
echo 45
if (2 > 3)
	echo 0
end
echo 46
if 2 < 3
	echo 1
end
echo 47
if (2 > 3)
	echo 0
end
echo 48
if 2 >= 2
	echo 1
end
echo 49
if 2 >= 3
	echo 0
end
echo 50
if 2 != 3
	echo 1
end
echo 51
if 2 != 2
	echo 0
end
echo 52
while 1 != 1
	echo 2
end
echo 53
while 2 != 2
	echo 1
end
