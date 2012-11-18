#!/usr/bin/ehi
Object.operator!!! = func: rhs
	echo('Calling operator!!! on ' + this + ' and ' + rhs)
	this + rhs
end

3 !!! 4
printvar (3 !!! 4 !!! 5)

Integer.operator** = rhs => pow(this, rhs)

echo(2 ** 3)
