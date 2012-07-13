#!/usr/bin/ehi
# Call the factorial function
include: 'factorial.eh'
if $argc == 2
	n = $argv->1
else
	n = 13
end
for $n count i
	echo factorial: $i
end
