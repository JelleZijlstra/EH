#!/usr/bin/ehi
# The default keyword, and some type juggling
switch $argc
	case 0
		echo 'That is impossible'
	case 1
		echo 'That is one'
	case 2
		echo 'That is two'
	default
		echo 'That is some other number'
	case 4
		echo 'That will not work'
end
set bar = given true
	case 2
		false
	case 0
		true
	default
		1..3
end
echo $bar
