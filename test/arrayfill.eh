#!/usr/bin/ehi
# Probably a useful library routine
Array.fill(n, f) = do
	out = []
	for i in n
		out.push(f i)
	end
	out
end

printvar(Array.fill(5, (input => input)))
printvar(Array.fill(10, (input => input * input)))
