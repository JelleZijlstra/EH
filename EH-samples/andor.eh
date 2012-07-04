#!/usr/bin/ehi
# Illustrate the AND and OR operators
bar := 3
foo := 0
printvar: @bool $foo
printvar: $bar and $foo
printvar: $bar && $foo
printvar: $bar or $foo
printvar: $bar || $foo
func test: n
	echo 'This is a function ' . $n
endfunc
false and test: 1
true and test: 2
false or test: 3
true or test: 4
# true
printvar: $foo xor $bar
# false
printvar: $bar xor $bar
# false
printvar: $foo xor 0
# This also illustrates a parser deficiency: the following lines generate a parser error without the semicolon, because the comment eats the following newline and thus there is no T_SEPARATOR.
printvar: 0 & 1; # 0
printvar: 0 | 1; # 1
printvar: 0 ^ 1; # 1
printvar: 4 % 2; # 0
