#!/usr/bin/ehi
# Calculate the Fibonacci sequence
echo 'Calculating the Fibonacci sequence'
$ na = 0
echo $na
$ nb = 1
echo $nb
# continue until we start overflowing
while $nb > 0
	$ tmp = $nb
	$ nb = $nb + $na
	$ na = $tmp
	echo $nb
endwhile
