#!/usr/bin/ehi
include '../lib/exception.eh'

# Casting to and from floats
printvar @float 3
rescue () => (printvar @float true)
printvar @int 3.2
printvar 3 == @int 3.0
printvar 3.0 == 3.0
printvar 3 == 3.0
