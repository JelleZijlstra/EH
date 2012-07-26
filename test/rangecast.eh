#!/usr/bin/ehi
# Test array-to-range conversion and similar obscure casts
r = 1..3
a = [1, 3]
printvar: @array r
printvar: @range a
