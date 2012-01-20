#!/usr/bin/ehi
# The Sieve of Eratosthenes, using ranges instead of integer counts (as in
# eratosthenes.eh). Illustrates that this is much easier to handle.
if $argc = 2
	set max = $argv->1
else
	set max = 128
end
set sieve = []
for 1..$max count i
	set sieve->$i = true
end
for 2..($max / 2) count i
	if $sieve->$i
		for 2..($max / $i) count j
			set sieve->($i * $j) = false
		end
	end
end
for 1..$max count i
	if $sieve->$i = true
		echo $i
	end
end
