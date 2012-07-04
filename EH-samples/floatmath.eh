#!/usr/bin/ehi
# Math with floats
pi := 3.14
e := 1.68
answer := 42
printvar: $pi + $e
printvar: $pi * $e
printvar: $pi / $e
printvar: $answer / $pi
printvar: $pi % $e
printvar: $pi & $e
printvar: $pi * $answer
printvar: $pi + $answer
printvar: $pi * (@float $answer)
