#!/usr/bin/ehi
# The Sieve of Eratosthenes
if argc == 2
	max = argv->1
else
	max = 128
end
sieve = []
for i in max
	sieve->(i + 1) = true
end
for i in (max / 2) - 1
	for j in (max / (i + 2)) - 1
		sieve->((i + 2) * (j + 2)) = false
	end
end
for i in max
	if sieve->(i + 1) == true
		echo(i + 1)
	end
end
