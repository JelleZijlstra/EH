#!/usr/bin/ehi
// CS50 Problem Set 1: greedy
if $argc == 2
	money = @int $argv->1
else
	money = 0
endif
while $money <= 0
	echo 'How much change is owed?'
	money = getinput: 10
endwhile

// get number of pennies
money = $money * 100
coins = 0

// get quarters
while $money > 25
	set coins++
	money = $money - 25
endwhile

// dimes
while $money > 10
	set coins++
	money = $money - 10
endwhile

// nickels
while $money > 5
	set coins++
	money = $money - 5
endwhile

// pennies
coins = $coins + $money

echo $coins
