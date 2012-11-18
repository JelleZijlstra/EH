#!/usr/bin/ehi

for i in 1..100
	if i % 15 == 0
		echo "FizzBuzz"
	elsif i % 3 == 0
		echo "Fizz"
	elsif i % 5 == 0
		echo "Buzz"
	else
		echo i
	end
end
