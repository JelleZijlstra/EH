#!/usr/bin/ehi

include: 'arrayfunc_lib.eh'

set arr = [1, -1, 0, 2, -2]
set arr = arrayfunc: $arr, func: in
	if $in > 0
		ret 1
	else 
		if $in < 0
			ret - 1
		else
			ret 0
		end
	end
end
printvar: $arr