// Calculate the Fibonacci sequence
echo 'Calculating the Fibonacci sequence'
$ na = 0
$ nb = 1
while 1
	$ tmp = $nb
	$ nb = $nb + $na
	$ na = $tmp
	echo $nb
endwhile
