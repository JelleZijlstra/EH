#!/usr/bin/ehi
# The Sieve of Eratosthenes
if $argc == 2
	max := $argv->1
else
	max := 128
end
sieve := []
for $max count i
	sieve->($i + 1) := true
end
for ($max / 2) - 1 count i
	for ($max / ($i + 2)) - 1 count j
		sieve->(($i + 2) * ($j + 2)) := false
	end
end
for $max count i
	if $sieve->($i + 1) == true
		echo $i + 1
	end
end
