#!/usr/bin/ehi
// CS50 Problem Set 1: greedy
if argc == 2
	money = @int argv->1
else
	money = 0
end
while money <= 0
	echo 'How much change is owed?'
	money = getinput()
end

// get number of pennies
money = money * 100
coins = 0

// get quarters
while money > 25
	coins++
	money = money - 25
end

// dimes
while money > 10
	coins++
	money = money - 10
end

// nickels
while money > 5
	coins++
	money = money - 5
end

// pennies
coins = coins + money

echo coins
