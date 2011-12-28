#!/usr/bin/ehi
# Illustrate the AND and OR operators
$ bar = 3
$ foo = 0
printvar: @bool $foo
printvar: $bar and $foo
printvar: $bar && $foo
printvar: $bar or $foo
printvar: $bar || $foo
func test: n
	echo 'This is a function ' + @string $n
endfunc
false and test: 1
true and test: 2
false or test: 3
true or test: 4
