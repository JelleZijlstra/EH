#!/usr/bin/ehi
include '../lib/exception.eh'

# More crazy casts

printvar @string 1..3
printvar @bool (File.new ())
rescue func: -> (printvar @bool func: n -> n)
rescue func: -> (printvar @range 3.14)
printvar @array null
