#!/usr/bin/ehi
# The Ackermann function
func ackermann: m, n
	if m == 0
		ret n + 1
	else
		if n == 0
			ret ackermann m - 1, 1
		else
			ret ackermann m - 1, (ackermann m, n - 1)
		end
	end
end

for m in 0..2
	for n in 0..(3 - m)
		echo 'A(' + m + ',' + n + ') = ' + (ackermann m, n)
	end
end
