#!/usr/bin/ehi
# For-in loops
arr = [1, 2]
for v in arr
	printvar v
end

arr2 = []
for i in arr
	arr2.push i
	printvar arr2
end

for i in 42..45
	printvar i
end
