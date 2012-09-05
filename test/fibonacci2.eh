#!/usr/bin/ehi
# Calculate the Fibonacci sequence with more consise code
echo 'Calculating the Fibonacci sequence'
a, b = 0, 1
# continue until we start overflowing
while b > 0
	echo b
	a, b = b, a + b
end
