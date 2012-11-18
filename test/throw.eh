#!/usr/bin/ehi
# Exceptions
funky = func: input
	if (input % 3) == 0
		throw "I don't like numbers that are divisible by three"
	else
		ret input
	end
end

trying = func: input
	try
		ret funky input
	catch
		echo 'Caught it'
		printvar exception
		ret 42
	finally
		echo 'I am getting called'
	end
end

printvar trying 2
printvar trying 3
