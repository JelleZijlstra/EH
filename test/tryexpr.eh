#!/usr/bin/ehi
# Test try-expressions

echo try
	3
catch
	4
finally
	5
end

echo try
	throw 42
catch if exception.isA Integer
	3
end
