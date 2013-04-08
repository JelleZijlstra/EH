#!/usr/bin/ehi
# The Sieve of Eratosthenes, using ranges instead of integer counts (as in
# eratosthenes.eh). Illustrates that this is much easier to handle.
if argc == 2
	max = argv->1
else
	max = 128
end
sieve = []
for i in 0..max
	sieve.push true
end
for i in 2..(max / 2)
	if sieve->i
		for j in 2..(max / i)
			sieve->(i * j) = false
		end
	end
end
for i in 1..max
	if sieve->i == true
		echo i
	end
end
