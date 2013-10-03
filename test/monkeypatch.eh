#!/usr/bin/ehi
# You can do this, but do you really want to?
String##operator* n = do
	if n < 1
		echo 'Invalid argument'
		ret null
	end
	out = ''
	for n
		out = out + this
	end
	out
end
printvar("true" * 3)
