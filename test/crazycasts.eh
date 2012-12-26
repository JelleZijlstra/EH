#!/usr/bin/ehi
include '../lib/array.eh'
include '../lib/exception.eh'

# Some crazy typecasts that EH supports
# [ 0 => 1, 1 => 2, ..., 41 => 42]
printvar((1..42).toArray())
# [ 0 => 'string']
rescue(() => printvar('string'.toArray()))
# [ 0 => 42]
rescue(() => printvar(42.toArray()))
# [ 0 => true]
rescue(() => printvar(true.toArray()))
# 1..42
printvar('1 to 42'.toRange())
# error
rescue(() => printvar(42.toRange()))
# error
rescue(() => printvar([1].toBool()))
# error
rescue(() => printvar([].toBool()))
