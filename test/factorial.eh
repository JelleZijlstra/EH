#!/usr/bin/ehi
# A factorial function
# Because ehi integers are internally 32-bit, this overflows for n > 12
func factorial: n
	# Guard against nonpositive input
	if n < 1
		ret false
	end
	if n == 1
		ret 1
	else
		# Parentheses are needed here because * has higher precedence than functions calls do.
		# Ideally those should have a higher precedence, but this runs into problems
		# because foo -1 would get interpreted as (foo) - 1.
		ret n * (factorial n - 1)
	end
end
