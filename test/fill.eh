#!/usr/bin/ehi
include '../lib/library.eh'
Array.fill = func: n, f
	out = []
	for i in 0..(n - 1)
		out->i = f i
	end
	out
end

printvar Array.fill 1, (x => x)
printvar Array.fill 25, (x => x * x)

