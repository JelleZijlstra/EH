#!/usr/bin/ehi
# Forgot to implement this at first
printvar: (1..3)->0
printvar: (1..3)->1
set var = 1..3
$ var->0 = 2
printvar: $var
$ var->1 = 4
printvar: $var
