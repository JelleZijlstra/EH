#!/usr/bin/ehi
foo = func: n
	n = n + 1
	echo n
	ret n
end
bar = 3
echo foo bar
echo bar
try
	echo foo () &bar
catch
	echo exception
end
echo bar
