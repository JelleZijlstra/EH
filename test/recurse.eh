#!/usr/bin/ehi
# Simple recursal. Was buggy in previous version of ehi.
rec = func: n
	if n == 0
		0
	else
		tmp = n
		rec(n - 1)
	end
end
rec 2
