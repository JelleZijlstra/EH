#!/usr/bin/ehi
# Illustrate uses of the break keyword
n = 0
while 1
	set n++
	echo n
	if (n > 2)
		break
	endif
endwhile
echo 'We got out of the infinite loop'

while 1
	echo 'Outer: ' + n
	set n++
	while 1
		echo 'Inner: ' + n
		set n++
		if (n > 8)
			echo 'Breaking 2'
			break 2
		endif
		if (n > 5)
			echo 'Breaking 1'
			break 1
		endif
	endwhile
endwhile

echo 'Will this ever get executed?'
for n count i
	if (i == 3) or (i == 5)
		continue
	endif
	echo i
endfor
