#!/usr/bin/ehi
# A factorial function
# Because ehi integers are internally 32-bit, this overflows for n > 12
func factorial: n
	# Guard against nonpositive input
	if n < 1
		throw(ArgumentError.new("Invalid argument", "factorial", n))
	elsif n == 1
		1
	else
		n * factorial(n - 1)
	end
end
