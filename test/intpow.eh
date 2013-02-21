#!/usr/bin/ehi

intpow(m, n) = if n == 0
	1
elsif n == 1
	m
else
	part = intpow(m, n / 2)
	if n % 2 == 1
		part * part * m
	else
		part * part
	end
end

echo(intpow(2, 16))
echo(intpow(3, 7))
