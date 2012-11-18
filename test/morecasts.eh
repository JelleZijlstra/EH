#!/usr/bin/ehi
include '../lib/exception.eh'

# More crazy casts

printvar @string 1..3
printvar @bool (File.new ())
rescue(() => printvar @bool (n => n))
rescue(() => printvar @range 3.14)
printvar @array null
