#!/usr/bin/ehi

# include EH standard library
include '../lib/library.eh'

# simple example of break
while true
	echo 3
	break
end

# and continue
private i = 0
while i < 6
	i += 1
	if i < 3
		continue
	end
	echo i
end

# multi-level break and continue
private const numberILike = 3
private const numberIDontLike = 7

# loop over a list of lists
private const numbers = (1::7::3::Nil)::(5::3::Nil)::(2::6::Nil)::Nil

for list in numbers
	for number in list
		# if we see this number, stop looking at the sublist
		if number == numberIDontLike
			continue 2
		# stop when we see this number
		elsif number == numberILike
			break 2
		end
		echo number
	end
end
