#!/usr/bin/ehi
# Illustrate uses of the break keyword
n = 0
while 1
	n++
	echo n
	if (n > 2)
		break
	end
end
echo 'We got out of the infinite loop'

while 1
	echo('Outer: ' + n)
	n++
	while 1
		echo('Inner: ' + n)
		n++
		if (n > 8)
			echo 'Breaking 2'
			break 2
		end
		if (n > 5)
			echo 'Breaking 1'
			break 1
		end
	end
end

echo 'Will this ever get executed?'
for i in n
	if (i == 3) or (i == 5)
		continue
	end
	echo i
end
