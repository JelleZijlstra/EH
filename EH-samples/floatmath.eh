#!/usr/bin/ehi
# Math with floats
set pi = 3.14
set e = 1.68
set answer = 42
printvar: $pi + $e
printvar: $pi * $e
printvar: $pi / $e
printvar: $answer / $pi
printvar: $pi % $e
printvar: $pi & $e
printvar: $pi * $answer
printvar: $pi + $answer
printvar: $pi * (@float $answer)
