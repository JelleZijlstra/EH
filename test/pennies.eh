#!/usr/bin/ehi
# Not yet compatible with ehi
# CS50 Problem Set 1 pennies
# loop to get days of month.
# Need complicated condition because we don't have "and"
days = -1
while days < 28 || days > 31
	put 'Days in month '
	days = getinput()
end

# loop to get number of pennies
pennies = 0
while pennies < 1
	put 'Pennies on first day '
	pennies = getinput()
end

# set up for calculation
byday = pennies
total = 0
days = days - 1

# calculate number of pennies
for days
	byday = byday * 2
	total = total + byday
end

# print amount of money (we don't care about floating-point imprecision here)
total = total / 100
echo total
