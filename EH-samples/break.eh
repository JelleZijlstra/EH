#!/usr/bin/ehi
# Illustrate uses of the break keyword
$ n = 0
while 1
	$ n++
	echo $n
	if $n > 2
		break
	endif
endwhile
echo 'We got out of the infinite loop'

while 1
	echo 'Outer: ' + @string $n
	$ n++
	while 1
		echo 'Inner: ' + @string $n
		$ n++
		if $n > 8
			echo 'Breaking 2'
			break 2
		endif
		if $n > 5
			echo 'Breaking 1'
			break 1
		endif
	endwhile
endwhile

echo 'Will this ever get executed?'
