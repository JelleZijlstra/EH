#!/usr/bin/ehi
# Functions are indeed objects
f = func: -> x
printvar: f:
$f.x = 3
printvar: f.x
printvar: f:
