#!/usr/bin/ehi
include 'arrayfunc_lib.eh'
arr = [1, 2]
printvar(arrayfunc(arr, (input => input + 1)))
# even shorter
printvar(arrayfunc(arr, 1.operator+))
