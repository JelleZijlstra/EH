#!/usr/bin/ehi
# Some crazy typecasts that EH supports
# [ 0 => 1, 1 => 2, ..., 41 => 42]
printvar: @array 1..42
# [ 0 => 'string']
printvar: @array 'string'
# [ 0 => 42]
printvar: @array 42
# [ 0 => true]
printvar: @array true
# 1..42
printvar: @range '1 to 42'
# error
printvar: @range 42
# error
printvar: @bool [1]
# error
printvar: @bool []
