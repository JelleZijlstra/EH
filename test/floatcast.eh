#!/usr/bin/ehi
include '../lib/exception.eh'

# Casting to and from floats
printvar(3.toFloat())
rescue(() => printvar(true.toFloat()))
printvar(3.2.toInteger())
printvar(3 == 3.0.toInteger())
printvar(3.0 == 3.0)
printvar(3 == 3.0)
