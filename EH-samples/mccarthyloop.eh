#!/usr/bin/ehi
# Implementation of the McCarthy function in EH
# This version uses more compact syntax that is allowed by ehi, but not by the 
# PHP interpreter.
func mccarthy: n
	if $n > 100
		ret $n - 10
	else
		ret mccarthy: mccarthy: $n + 11
	endif
endfunc

# Use $argv[1] if available
if $argc > 2
	echo 'Usage: ./mccarthy.eh [n]'
	ret 1
endif
if $argc = 2
	$ input = @int $argv->1
else
	$ input = getinput:
endif
for $input count i
	echo mccarthy: $i
endfor
ret 0
