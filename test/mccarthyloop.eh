#!/usr/bin/ehi
# Implementation of the McCarthy function in EH
# This version uses more compact syntax that is allowed by ehi, but not by the
# PHP interpreter.
func mccarthy: n
	if n > 100
		ret n - 10
	else
		ret mccarthy(mccarthy(n + 11))
	end
end

# Use argv[1] if available
if argc > 2
	echo 'Usage ./mccarthy.eh [n]'
	ret 1
end
if argc == 2
	input = argv->1.toInteger()
else
	input = getinput()
end
for i in input
	echo(mccarthy i)
end
ret 0
