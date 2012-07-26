#!/usr/bin/ehi
f = func:
	$quit
	$echo 42
end
# expect nothing to be printed, because quit exits immediately
printvar: f:
