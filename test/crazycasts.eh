#!/usr/bin/ehi
include '../lib/array.eh'
include '../lib/exception.eh'

# Some crazy typecasts that EH supports
# [ 0 => 1, 1 => 2, ..., 41 => 42]
printvar @array 1..42
# [ 0 => 'string']
rescue(() => printvar @array 'string')
# [ 0 => 42]
rescue(() => printvar @array 42)
# [ 0 => true]
rescue(() => printvar @array true)
# 1..42
printvar(@range '1 to 42')
# error
rescue(() => printvar @range 42)
# error
rescue(() => printvar @bool [1])
# error
rescue(() => printvar @bool [])
