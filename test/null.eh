#!/usr/bin/ehi
# Illustrates the use of null variables and the printvar: library function
bar = null
echo $bar
foo = ['foo', null, 'test']
echo $foo -> 1
# Casting null to a string gives an empty string
baz = @string $bar
echo $baz
# Casting null to an int gives 0 and throws an error
ban = @int $bar
echo $ban
printvar: $ban
printvar: $foo