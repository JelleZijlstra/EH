#!/usr/bin/ehi
f := func:
	quit
	echo 42
end
# expect 0, because quit == ret 0
printvar: $f:
