// CS50 Problem Set 1: greedy
$ money = 0
while $money <= 0
	echo How much change is owed?
	$ money = getinput: 10
endwhile

// get number of pennies
$ money = $money * 100
$ coins = 0

// get quarters
while $money > 25
	$ coins++
	$ money = $money - 25
endwhile

// dimes
while $money > 10
	$ coins++
	$ money = $money - 10
endwhile

// nickels
while $money > 5
	$ coins++
	$ money = $money - 5
endwhile

// pennies
$ coins = $coins + $money

echo $coins
