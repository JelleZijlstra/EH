#!/usr/bin/ehi
a = 3
b = a
c = ['foo' => 1]
for (a, d) in c
	# foo
	printvar a
	# 1
	printvar d
end
# foo
printvar a
# 3
printvar b
# [foo => 1]
printvar c
# 1
printvar d
