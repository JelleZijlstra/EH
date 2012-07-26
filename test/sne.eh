#!/usr/bin/ehi
# The strict not-equal operator
# All these $echo true if ehi works as expected, but false if it doesn't.
if 3 != '3'
	$echo true
else
	$echo false
end

if 3 != 3
	$echo false
else
	$echo true
end

if 1..3 != 1..3
	$echo false
else
	$echo true
end

if 3.0 != 3
	$echo true
else
	$echo false
end

if 1.0 != true
	$echo true
else
	$echo false
end
