#!/usr/bin/ehi
# Basic EH loop
# set input
in := 10
# counter variable
total := 1
for $in count i
	total := $total * $in
	set in--
end
echo $total
