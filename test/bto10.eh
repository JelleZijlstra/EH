#!/usr/bin/ehi
# Convert numeric strings in a given base to integers
# Doesn't take into account many greater and lesser subtleties, but it
# gets the core of the job done.
func bto10: input, base
	getn = func: input, n
		char = input.charAtPosition n
		if char >= 48 and char <= 57
			ret char - 48
		end
		# lowercase letters
		if (char >= 97) and (char <= 122)
			ret char - 87
		end
		# uppercase letters
		if char >= 65 and char <= 90
			ret char - 55
		end
		ret false
	end
	out = 0
	len = input.length()
	for len count i
		value = getn input, i
		out = out + value * (pow base, (len - 1 - i))
	end
	ret out
end

# 42
echo (bto10 "2a", 16)
# 44
echo (bto10 "35", 13)
