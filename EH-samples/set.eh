#!/usr/bin/ehi
# Testing "set" as an alternative for "$ "
set a = 3
echo $a
set b = [ 1, 2, 3, 4]
set b -> 4 = 5
echo $b->4
