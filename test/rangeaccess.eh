#!/usr/bin/ehi
# Forgot to implement this at first
printvar((1..3)->0)
printvar((1..3)->1)
var = 1..3
var = 2..(var->1)
printvar var
var = (var->1)..4
printvar var
