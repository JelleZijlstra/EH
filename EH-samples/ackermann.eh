#!/usr/bin/ehi
# The Ackermann function
func ackermann: m, n
	if $m == 0
		ret $n + 1
	else
		if $n == 0
			ret ackermann: $m - 1, 1
		else
			ret ackermann: $m - 1, (ackermann: $m, $n - 1)
		end
	end
end

for 0..2 count m
	for 0..(3 - $m) count n
		echo 'A(' + $m + ',' + $n + ') = ' + (ackermann: $m, $n)
	end
end
