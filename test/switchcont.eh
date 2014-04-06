#!/usr/bin/ehi
# Using the continue keyword in switches
match argc
	case 1
		echo 'I got no arguments'
	case 2
		echo 'I got two'
	case _ when argc.isA Integer
		echo 'Will this get executed?'
	case "2"
		echo 'And this?'
end
if 2.toString() == "2"
	echo 'They are loosely the same'
end
