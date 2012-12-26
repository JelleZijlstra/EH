#!/usr/bin/ehi

private counter = 0

# Basic while loop, prints numbers from 0 to 4
while counter < 5
	echo counter
	counter += 1
end

counter = 0

# This would be an infinite loop...
while true
	echo counter
	counter += 1
	if counter == 5
		# ... if this break statement was not here
		break
	end
end
