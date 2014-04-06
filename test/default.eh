#!/usr/bin/ehi
# The default keyword, and some type juggling
match argc
	case 0
		echo 'That is impossible'
	case 1
		echo 'That is one'
	case 2
		echo 'That is two'
	case _
		echo 'That is some other number'
	case 4
		echo 'That will not work'
end
bar = match true
	case 2
		false
	case 0
		true
	case _
		1..3
end
echo bar
