// CS50 Problem Set 1: pennies

// loop to get days of month. 
// Need complicated condition because we don't have "and"
$ goin = 1
while $goin 
	put 'Days in month: '
	$ days = getinput: 
	if $days < 28
		$ goin = 1
	else
		if $days > 31
			$ goin = 1
		else
			$ goin = 0
		endif
	endif
endwhile

// loop to get number of pennies
$ pennies = 0
while $pennies < 1
	put 'Pennies on first day: '
	$ pennies = getinput:
endwhile

// set up for calculation
$ byday = $pennies
$ total = 0
$ days = $days - 1

// calculate number of pennies
for $days
	$ byday = $byday * 2
	$ total = $total + $byday
endfor

// print amount of money (we don't care about floating-point imprecision here)
$ total = $total / 100
put $
echo $total
