#!/usr/bin/ehi
# Test error handling when range members have different types
try
	a = 1.."foo"
catch
	printvar exception
end
# And now don't catch it
42..[]
