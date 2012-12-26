#!/usr/bin/ehi

private x = 3
private y = 4

# Simple if block
if x == 3
	echo "x equals 3"
end

# else is executed if the if block fails
if y == 3
	echo "y equals 3"
else
	echo "y does not equal 3"
end

# We can also add any number of elseif blocks
if y == 3
	echo "y equals 3"
elsif y == 4
	echo "y equals 4"
else
	echo "y does not equal 3"
end

# An if block may also be used as an expression
echo if y == 4
	"y equals 4"
else
	"y does not equal 4"
end
