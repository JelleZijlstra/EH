#!/usr/bin/ehi

f1 = func: args -> (printvar args)
f2 = func: a, b
	printvar a
	printvar b
end
f3 = func: a, b, c
	printvar a
	printvar b
	printvar c
end

f1(1, 2)
f2(1, 2)
f3(1, 2, 3)
