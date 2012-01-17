#!/usr/bin/ehi
# As much looping as possible
for [1, 2, 3, 4, 5] as value
	echo 'before ' + @string $value
	for $value count i
		echo $i
		if $i = 2 && $value < 4
			continue 2
		end
		if $i = 4
			break 2
		end
		echo 'inner after ' + @string $i
	end
	echo 'after ' + @string $value
end
