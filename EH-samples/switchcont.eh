#!/usr/bin/ehi
# Using the continue keyword in switches
switch $argc
	case 1
		echo 'I got no arguments'
		continue
	case 2
		echo 'I got two'
	case is_int
		echo 'Will this get executed?'
	case "2"
		echo 'And this?'
end
if 2.toString: == "2"
	echo 'They are loosely the same'
end
