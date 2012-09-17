#!/usr/bin/ehi
# For-in loops
arr = [1, 2]
for (k, v) in arr
	printvar k, v
end

arr2 = []
for (i, arr2->i) in arr
	printvar arr2
end

for i in 42..45
	printvar i
end
