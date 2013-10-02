#!/usr/bin/ehi
include '../lib/exception.eh'

# Functions are indeed objects, they just don't allow member assignment
f() = scope.x
rescue f
f.x = 3
printvar(f.x)
printvar(f())
