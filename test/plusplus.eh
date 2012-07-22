#!/usr/bin/ehi
# To illustrate the use of the ++ operator
foo = 2
# Expect 2
echo foo
set foo++
# Expect 3
echo $foo
set foo--
# Expect 2
echo foo
bar = [ 1, 2, 3 ]
# Expect 2
echo $bar->1
set $bar->1--
# Expect 1
echo $bar->1
set $bar->1++
# Expect 2
echo $bar->1
