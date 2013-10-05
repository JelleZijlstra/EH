#!/usr/bin/ehi
include '../lib/library.eh'

code = raw(Array##map f = 42)->0

printvar code
printvar(Macro.optimize code)
