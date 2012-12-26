#!/usr/bin/ehi
# Implementation of the McCarthy function in EH
func mccarthy: n
	if n > 100
		retv = n - 10
		ret retv
	else
		arg = n + 11
		arg = mccarthy arg
		arg = mccarthy arg
		ret arg
	end
end

# Use argv[1] if available
if argc > 2
	echo 'Usage ./mccarthy.eh [n]'
	ret 1
end
if argc == 2
	input = argv->1.toInt()
else
	input = getinput ()
end
res = mccarthy input
echo res
ret 0
