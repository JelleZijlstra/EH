#!/usr/bin/ehi
# More crazy casts

printvar: @string 1..3
printvar: @bool (File.new:)
printvar: @bool func: n; ret n; end
printvar: @range 3.14
printvar: @array null
