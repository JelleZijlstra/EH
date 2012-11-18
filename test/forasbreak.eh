#!/usr/bin/ehi
# As much looping as possible
for ((), value) in [1, 2, 3, 4, 5]
	echo('before ' + value)
	for i in value
		echo i
		if i == 2 && value < 4
			continue 2
		end
		if i == 4
			break 2
		end
		echo('inner after ' + i)
	end
	echo('after ' + value)
end
