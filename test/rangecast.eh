#!/usr/bin/ehi
include '../lib/array.eh'
# Test array-to-range conversion and similar obscure casts
r = 1..3
a = [1, 3]
printvar(r.toArray())
printvar(a.toRange())
