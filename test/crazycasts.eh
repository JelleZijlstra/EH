#!/usr/bin/ehi
include '../lib/array.eh'
include '../lib/exception.eh'

# Some crazy typecasts that EH supports
# [ 0 => 1, 1 => 2, ..., 41 => 42]
printvar @array 1..42
# [ 0 => 'string']
rescue func: -> (printvar @array 'string')
# [ 0 => 42]
rescue func: -> (printvar @array 42)
# [ 0 => true]
rescue func: -> (printvar @array true)
# 1..42
printvar @range '1 to 42'
# error
rescue func: -> (printvar @range 42)
# error
rescue func: -> (printvar @bool [1])
# error
rescue func: -> (printvar @bool [])
