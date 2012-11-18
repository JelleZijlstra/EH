#!/usr/bin/ehi

include 'arrayfunc_lib.eh'

arr = [1, (-1), 0, 2, (-2)]
arr2 = arrayfunc(arr, func: input
	if input > 0
		ret 1
	else 
		if input < 0
			ret -1
		else
			ret 0
		end
	end
end)
printvar arr2
arr3 = arrayfunc(arr, 0.operator<=>)
printvar arr3
